#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Camera/Follow/State/Interface/FollowCameraIState.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	FollowCameraSwitchAllyState class
//	スタンバイ状態から味方にカメラを切り替える状態
//============================================================================
class FollowCameraSwitchAllyState :
	public FollowCameraIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraSwitchAllyState() = default;
	~FollowCameraSwitchAllyState() = default;

	void Enter(FollowCamera& followCamera) override;

	void Update(FollowCamera& followCamera)  override;

	void Exit() override;

	// imgui
	void ImGui(const FollowCamera& followCamera) override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data)  override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------
	
	// parameters
	float lerpTimer_; // 遷移経過時間
	float lerpTime_;  // 遷移時間
	EasingType lerpEasingType_;

	Vector3 offsetTranslation_; // 追従相手との距離
	Vector3 interTarget_;       // 追従中間target位置
	float lerpRate_;            // 補間割合

	std::optional<float> startFovY_; // 開始画角
	float targetFovY_;               // 目標画角
};