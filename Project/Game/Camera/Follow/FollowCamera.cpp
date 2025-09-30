#include "FollowCamera.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Random/RandomGenerator.h>
#include <Engine/MathLib/MathUtils.h>

//============================================================================
//	FollowCamera classMethods
//============================================================================

void FollowCamera::Init() {

	displayFrustum_ = false;

	// json適応
	ApplyJson();

	stateController_ = std::make_unique<FollowCameraStateController>();
	stateController_->Init(*this);

	// 行列更新
	BaseCamera::UpdateView();
}

void FollowCamera::SetScreenShake(bool isShake) {

	if (isShake) {

		// 状態を設定する
		stateController_->SetOverlayState(*this, FollowCameraOverlayState::Shake);
	} else {

		// shakeを止める
		stateController_->ExitOverlayState(FollowCameraOverlayState::Shake);
	}
}

void FollowCamera::SetParry(bool isParry) {

	if (isParry) {

		// 状態を設定する
		stateController_->SetOverlayState(*this, FollowCameraOverlayState::Parry);
	} else {

		// parryを止める
		stateController_->ExitOverlayState(FollowCameraOverlayState::Parry);
	}
}

void FollowCamera::SetParryAttack(bool isParry) {

	if (isParry) {

		// 状態を設定する
		stateController_->SetOverlayState(*this, FollowCameraOverlayState::ParryAttack);
	} else {

		// parryを止める
		stateController_->ExitOverlayState(FollowCameraOverlayState::ParryAttack);
	}
}

void FollowCamera::SetTarget(FollowCameraTargetType type, const Transform3D& target) {

	stateController_->SetTarget(type, target);
}

void FollowCamera::SetState(FollowCameraState state) {

	stateController_->SetState(state);
}

void FollowCamera::Update() {

	// エディターで更新しているときは処理しない
	if (isUpdateEditor_) {
		return;
	}

	// 状態の更新
	stateController_->Update(*this);

	// 行列更新
	BaseCamera::UpdateView();
}

void FollowCamera::ImGui() {

	ImGui::SetWindowFontScale(0.8f);
	ImGui::PushItemWidth(itemWidth_);

	if (ImGui::BeginTabBar("FollowCameraTabs")) {
		if (ImGui::BeginTabItem("Init")) {
			if (ImGui::Button("Save Json", ImVec2(itemWidth_, 32.0f))) {

				SaveJson();
			}

			ImGui::DragFloat3("translation", &transform_.translation.x, 0.01f);
			ImGui::DragFloat3("rotation", &transform_.eulerRotate.x, 0.01f);

			ImGui::DragFloat("fovY", &fovY_, 0.01f);
			ImGui::DragFloat("nearClip", &nearClip_, 0.001f);
			ImGui::DragFloat("farClip", &farClip_, 1.0f);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("State")) {

			stateController_->ImGui(*this);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::SetWindowFontScale(1.0f);
	ImGui::PopItemWidth();
}

void FollowCamera::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Camera/Follow/initParameter.json", data)) {
		return;
	}

	fovY_ = JsonAdapter::GetValue<float>(data, "fovY_");
	nearClip_ = JsonAdapter::GetValue<float>(data, "nearClip_");
	farClip_ = JsonAdapter::GetValue<float>(data, "farClip_");
}

void FollowCamera::SaveJson() {

	Json data;

	data["fovY_"] = fovY_;
	data["nearClip_"] = nearClip_;
	data["farClip_"] = farClip_;

	JsonAdapter::Save("Camera/Follow/initParameter.json", data);
}