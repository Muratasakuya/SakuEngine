#include "FollowCameraStateController.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

// inputDevice
#include <Game/Camera/Follow/Input/Device/FollowCameraKeyInput.h>
#include <Game/Camera/Follow/Input/Device/FollowCameraGamePadInput.h>

// state
#include <Game/Camera/Follow/State/States/FollowCameraFollowState.h>
#include <Game/Camera/Follow/State/States/FollowCameraSwitchAllyState.h>
#include <Game/Camera/Follow/State/States/FollowCameraAllyAttackState.h>
#include <Game/Camera/Follow/State/States/FollowCameraStunAttackState.h>
#include <Game/Camera/Follow/State/States/FollowCameraShakeState.h>
#include <Game/Camera/Follow/State/States/FollowCameraParryState.h>
#include <Game/Camera/Follow/State/States/FollowCameraParryAttackState.h>

// imgui
#include <imgui.h>

//============================================================================
//	FollowCameraStateController classMethods
//============================================================================

void FollowCameraStateController::Init(FollowCamera& owner) {

	// 入力クラスを初期化
	Input* input = Input::GetInstance();
	inputMapper_ = std::make_unique<InputMapper<FollowCameraInputAction>>();
	inputMapper_->AddDevice(std::make_unique<FollowCameraGamePadInput>(input));

#ifdef _RELEASE
	//inputMapper_->AddDevice(std::make_unique<FollowCameraKeyInput>(input));
#endif
	// 各状態を初期化
	states_.emplace(FollowCameraState::Follow, std::make_unique<FollowCameraFollowState>(owner.GetFovY()));
	states_.emplace(FollowCameraState::SwitchAlly, std::make_unique<FollowCameraSwitchAllyState>());
	states_.emplace(FollowCameraState::AllyAttack, std::make_unique<FollowCameraAllyAttackState>(owner.GetFovY()));
	states_.emplace(FollowCameraState::StunAttack, std::make_unique<FollowCameraStunAttackState>());
	overlayStates_.emplace(FollowCameraOverlayState::Shake, std::make_unique<FollowCameraShakeState>());
	overlayStates_.emplace(FollowCameraOverlayState::Parry, std::make_unique<FollowCameraParryState>(owner.GetFovY()));
	overlayStates_.emplace(FollowCameraOverlayState::ParryAttack, std::make_unique<FollowCameraParryAttackState>());

	// json適応
	ApplyJson();

	// inputを設定
	SetInputMapper();
	// 状態間の値の共有
	SetStateValue();

	// 最初の状態を設定
	current_ = FollowCameraState::Follow;
	requested_ = FollowCameraState::Follow;
	ChangeState(owner);
}

void FollowCameraStateController::SetTarget(FollowCameraTargetType type, const Transform3D& target) {

	// 各状態にtargetをセット
	for (const auto& state : std::views::values(states_)) {

		state->SetTarget(type, target);
	}
	for (const auto& state : std::views::values(overlayStates_)) {

		state->SetTarget(type, target);
	}
}

void FollowCameraStateController::SetOverlayState(FollowCamera& owner, FollowCameraOverlayState state) {

	// 依頼された状態を設定
	overlayState_ = state;

	// 再生中なら終わりにしてリセットさせる
	if (!overlayStates_[state]->GetCanExit()) {

		overlayStates_[state]->Exit();
	}
	overlayStates_[state]->Enter(owner);
}

void FollowCameraStateController::ExitOverlayState(FollowCameraOverlayState state) {

	// 強制終了
	overlayState_ = std::nullopt;
	overlayStates_[state]->Exit();
}

void FollowCameraStateController::SetStateValue() {

	// 状態間の値の共有(値ずれを防ぐため)
	static_cast<FollowCameraParryState*>(overlayStates_.at(FollowCameraOverlayState::Parry).get())->SetStartOffsetTranslation(
		static_cast<FollowCameraFollowState*>(states_.at(FollowCameraState::Follow).get())->GetOffsetTranslation());

	static_cast<FollowCameraParryAttackState*>(overlayStates_.at(FollowCameraOverlayState::ParryAttack).get())->SetStartOffsetTranslation(
		static_cast<FollowCameraParryState*>(overlayStates_.at(FollowCameraOverlayState::Parry).get())->GetCurrentOffset());
}

void FollowCameraStateController::SetInputMapper() {

	// 各状態にinputをセット
	for (const auto& state : std::views::values(states_)) {

		state->SetInputMapper(inputMapper_.get());
	}
}

void FollowCameraStateController::Update(FollowCamera& owner) {

	// 遷移状況の確認
	CheckExitOverlayState();

	// 何か設定されていれば遷移させる
	if (requested_.has_value()) {

		ChangeState(owner);
	}

	// 現在の状態を更新
	if (FollowCameraIState* state = states_[current_].get()) {

		state->Update(owner);
	}
	if (overlayState_.has_value()) {

		overlayStates_[overlayState_.value()]->Update(owner);
	}
}

bool FollowCameraStateController::Request(FollowCameraState state) {

	// 現在の状態と同じなら何もしない
	if (state == current_ && !states_.at(current_)->GetCanExit()) {
		return false;
	}

	// 遷移可能か、現在の状態が終了可能かチェック
	if (states_.at(current_)->GetCanExit()) {

		requested_ = state;
	}
	return true;
}

void FollowCameraStateController::ChangeState(FollowCamera& owner) {

	// 同じ状態なら遷移させない
	if (requested_.value() == current_) {
		requested_ = std::nullopt;
		return;
	}

	// 現在の状態の終了処理
	if (auto* currentState = states_[current_].get()) {

		currentState->Exit();
	}

	// 次の状態を設定する
	current_ = requested_.value();

	// 次の状態を初期化する
	if (auto* currentState = states_[current_].get()) {

		currentState->Enter(owner);
	}
}

void FollowCameraStateController::CheckExitOverlayState() {

	// 何も設定されていなければ処理しない
	if (!overlayState_.has_value()) {
		return;
	}

	auto state = overlayState_.value();
	if (!overlayStates_[state]->GetCanExit()) {
		return;
	}

	// Parryが終わる時にオフセット距離をセットする
	if (state == FollowCameraOverlayState::Parry) {

		const auto& follow = static_cast<FollowCameraFollowState*>(
			states_[FollowCameraState::Follow].get());
		const auto& parry = static_cast<FollowCameraParryState*>(
			overlayStates_[state].get());
		follow->SetOffsetTranslation(parry->GetCurrentOffset());
	} else if (state == FollowCameraOverlayState::ParryAttack) {

		const auto& follow = static_cast<FollowCameraFollowState*>(
			states_[FollowCameraState::Follow].get());
		const auto& parryAttack = static_cast<FollowCameraParryAttackState*>(
			overlayStates_[state].get());
		follow->SetOffsetTranslation(parryAttack->GetCurrentOffset());
	}

	overlayStates_[state]->Exit();
	overlayState_.reset();
}

void FollowCameraStateController::ImGui(FollowCamera& owner) {

	if (ImGui::Button("Save##StateJson")) {
		SaveJson();
	}

	ImGui::SeparatorText("State");
	EnumAdapter<FollowCameraState>::Combo("##StateCombo", &editingState_);
	states_[editingState_]->ImGui(owner);

	ImGui::SeparatorText("OverlayState");
	EnumAdapter<FollowCameraOverlayState>::Combo("##OverlayStateCombo", &editingOverlayState_);
	ImGui::Text(std::format("overlayHasValue: {}", overlayState_.has_value()).c_str());
	overlayStates_[editingOverlayState_]->ImGui(owner);
	if (ImGui::Button("Apply OverlayState")) {

		SetOverlayState(owner, editingOverlayState_);
	}
}

void FollowCameraStateController::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck(kStateJsonPath_, data)) {
		return;
	}

	for (const auto& [state, ptr] : states_) {

		ptr->ApplyJson(data[EnumAdapter<FollowCameraState>::ToString(state)]);
	}
	for (const auto& [state, ptr] : overlayStates_) {

		ptr->ApplyJson(data[EnumAdapter<FollowCameraOverlayState>::ToString(state)]);
	}
}

void FollowCameraStateController::SaveJson() {

	Json data;
	for (const auto& [state, ptr] : states_) {

		ptr->SaveJson(data[EnumAdapter<FollowCameraState>::ToString(state)]);
	}
	for (const auto& [state, ptr] : overlayStates_) {

		ptr->SaveJson(data[EnumAdapter<FollowCameraOverlayState>::ToString(state)]);
	}

	JsonAdapter::Save(kStateJsonPath_, data);
}