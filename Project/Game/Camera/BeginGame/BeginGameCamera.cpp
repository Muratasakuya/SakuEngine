#include "BeginGameCamera.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Camera/3D/Camera3DEditor.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	BeginGameCamera classMethods
//============================================================================

void BeginGameCamera::Init() {

	displayFrustum_ = true;
	isStarted_ = false;

	Camera3DEditor::GetInstance()->LoadAnimFile("beginGameCameraFirst.json");
	Camera3DEditor::GetInstance()->LoadAnimFile("beginGameCameraSecond.json");

	// json適応
	ApplyJson();
}

bool BeginGameCamera::IsFinished() const {

	// フラグがたっていれば遷移させない
	if (disableTransition_) {
		return false;
	}

	bool isFinished = Camera3DEditor::GetInstance()->IsAnimationFinished("beginGameCameraSecond"); 

	// カメラアニメーションが終了したか
	return isFinished;
}

void BeginGameCamera::StartAnimation() {

	// 最初のアニメーション開始
	Camera3DEditor::GetInstance()->StartAnim("beginGameCameraFirst", false, false);
	isStarted_ = true;
}

void BeginGameCamera::Update() {

	if (!isStarted_) {
		return;
	}

	// 最初のアニメーションが終了したら次のアニメーションを開始
	if (Camera3DEditor::GetInstance()->IsAnimationFinished("beginGameCameraFirst")) {

		Camera3DEditor::GetInstance()->StartAnim("beginGameCameraSecond", false, false);
		// この時点でアニメーションの開始フラグをfalseにする
		isStarted_ = false;
	}
}

void BeginGameCamera::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	ImGui::Checkbox("updateDebugView", &updateDebugView_);
	ImGui::Checkbox("disableTransition", &disableTransition_);
}

void BeginGameCamera::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Camera/BeginGame/animationParam.json", data)) {
		return;
	}

}

void BeginGameCamera::SaveJson() {

	Json data;

	JsonAdapter::Save("Camera/BeginGame/animationParam.json", data);
}