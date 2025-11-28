#include "SubPlayerStateController.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

// state
#include <Game/Objects/GameScene/SubPlayer/State/States/SubPlayerInactiveState.h>
#include <Game/Objects/GameScene/SubPlayer/State/States/SubPlayerPunchAttackState.h>

//============================================================================
//	SubPlayerStateController classMethods
//============================================================================

void SubPlayerStateController::Init() {

	// 各状態の初期化
	states_.emplace(SubPlayerState::Inactive, std::make_unique<SubPlayerInactiveState>());
	states_.emplace(SubPlayerState::PunchAttack, std::make_unique<SubPlayerPunchAttackState>());

	// json適応
	ApplyJson();

	// 最初の状態を設定
	requestState_ = SubPlayerState::Inactive;
	ChangeState(true);
}

void SubPlayerStateController::SetBossEnemy(const BossEnemy* bossEnemy) {

	// 各状態にボス敵を設定
	for (const auto& state : std::views::values(states_)) {

		state->SetBossEnemy(bossEnemy);
	}
}

void SubPlayerStateController::SetParts(GameObject3D* body, GameObject3D* rightHand, GameObject3D* leftHand) {

	// 各状態にパーツを設定
	for (const auto& state : std::views::values(states_)) {

		state->SetBody(body);
		state->SetRightHand(rightHand);
		state->SetLeftHand(leftHand);
	}
}

void SubPlayerStateController::Update() {

	// 状態の切り替え処理
	ChangeState(false);

	// 現在の状態を更新
	if (SubPlayerIState* state = states_[current_].get()) {

		state->Update();
	}
	// 常に更新する処理
	for (const auto& state : std::views::values(states_)) {

		state->UpdateAlways();
	}

	// 状態終了チェック
	CheckCanExit();
}

void SubPlayerStateController::ChangeState(bool isForce) {

	// 状態がリクエストされていなければ処理しない
	if (!requestState_.has_value()) {
		return;
	}

	// 同じ状態なら切り替えさせない、ただし強制切り替えの場合は除く
	if (!isForce && current_ == requestState_.value()) {
		requestState_ = std::nullopt;
		return;
	}

	// 現在の状態を終了
	states_[current_]->Exit();

	// 新しい状態に切り替え
	current_ = requestState_.value();
	requestState_ = std::nullopt;

	// 新しい状態を開始
	states_[current_]->Enter();
}

void SubPlayerStateController::CheckCanExit() {

	// 現在の状態が終了可能かチェック
	if (SubPlayerIState* state = states_[current_].get()) {
		if (state->CanExit()) {

			// 終了可能なら非アクティブ状態に遷移させる
			requestState_ = SubPlayerState::Inactive;
		}
	}
}

void SubPlayerStateController::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	EnumAdapter<SubPlayerState>::Combo("Edit State", &editState_);
	if (EnumAdapter<SubPlayerState>::Combo("Edit Request State", &editRequestState_)) {

		// リクエスト状態を設定
		requestState_ = editRequestState_;
		ChangeState(true);
	}

	ImGui::Separator();

	if (SubPlayerIState* state = states_[editState_].get()) {

		state->ImGui();
	}
}

void SubPlayerStateController::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("SubPlayer/State/stateParameter.json", data)) {
		return;
	}

	// 各状態にjson適応
	for (const auto& [state, ptr] : states_) {

		// 存在しないキーでは処理しない
		const auto& key = EnumAdapter<SubPlayerState>::ToString(state);
		if (!data.contains(key)) {
			continue;
		}
		ptr->ApplyJson(data[key]);
	}
}

void SubPlayerStateController::SaveJson() {

	Json data;

	// 各状態をjson保存
	for (const auto& [state, ptr] : states_) {

		ptr->SaveJson(data[EnumAdapter<SubPlayerState>::ToString(state)]);
	}
	JsonAdapter::Save("SubPlayer/State/stateParameter.json", data);
}