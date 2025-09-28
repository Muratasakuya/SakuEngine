#include "EndGameCamera.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	EndGameCamera classMethods
//============================================================================

void EndGameCamera::Init() {

	displayFrustum_ = true;

	// json適応
	ApplyJson();
}

bool EndGameCamera::IsFinished() const {

	// フラグがたっていれば遷移させない
	if (disableTransition_) {
		return false;
	}
	return currentState_ == State::Finished;
}

void EndGameCamera::Update() {

	switch (currentState_) {
	case EndGameCamera::State::Update:

		UpdateAnimation();
		break;
	case EndGameCamera::State::Finished:
		break;
	}

	// 行列更新
	BaseCamera::UpdateView();
}

void EndGameCamera::UpdateAnimation() {

	// 時間を進める
	animationTimer_.Update();

	// 座標を補間
	transform_.translation = Vector3::Lerp(
		startPos_, targetPos_, animationTimer_.easedT_);

	// 補間が終了したら次に進める
	if (animationTimer_.IsReached()) {

		animationTimer_.Reset();
		currentState_ = State::Finished;
	}
}

void EndGameCamera::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	ImGui::Checkbox("updateDebugView", &updateDebugView_);
	ImGui::Checkbox("disableTransition", &disableTransition_);
	EnumAdapter<State>::Combo("state", &currentState_);

	ImGui::DragFloat3("startPos", &startPos_.x, 0.1f);
	ImGui::DragFloat3("targetPos", &targetPos_.x, 0.1f);

	animationTimer_.ImGui("Animation");
}

void EndGameCamera::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Camera/EndGame/animationParam.json", data)) {
		return;
	}

	animationTimer_.FromJson(data["AnimationTimer"]);
	startPos_ = Vector3::FromJson(data["startPos_"]);
	targetPos_ = Vector3::FromJson(data["targetPos_"]);
}

void EndGameCamera::SaveJson() {

	Json data;

	animationTimer_.ToJson(data["AnimationTimer"]);
	data["startPos_"] = startPos_.ToJson();
	data["targetPos_"] = targetPos_.ToJson();

	JsonAdapter::Save("Camera/EndGame/animationParam.json", data);
}