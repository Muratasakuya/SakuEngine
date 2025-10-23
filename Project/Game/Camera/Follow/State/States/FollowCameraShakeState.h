#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Camera/Follow/State/Interface/FollowCameraIState.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	FollowCameraShakeState class
//	画面シェイク処理を行う状態
//============================================================================
class FollowCameraShakeState :
	public FollowCameraIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraShakeState();
	~FollowCameraShakeState() = default;

	void Enter(FollowCamera& followCamera) override;

	void Update(FollowCamera& followCamera)  override;

	void Exit()  override;

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

	float shakeXZIntensity_;      // シェイクのXZ強度
	float shakeOffsetYIntensity_; // シェイクのオフセットY強度
	float shakeTime_;             // シェイクの時間
	float shakeTimer_;            // シェイクの経過時間
	EasingType shakeEasingType_;  // シェイクのイージングタイプ
};