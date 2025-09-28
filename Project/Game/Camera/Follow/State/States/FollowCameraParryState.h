#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Camera/Follow/State/Interface/FollowCameraIState.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	FollowCameraParryState class
//============================================================================
class FollowCameraParryState :
	public FollowCameraIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraParryState(float startFovY) :startFovY_(startFovY) {};
	~FollowCameraParryState() = default;

	void Enter(FollowCamera& followCamera) override;

	void Update(FollowCamera& followCamera)  override;

	void Exit()  override;

	// imgui
	void ImGui(const FollowCamera& followCamera) override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data)  override;

	//--------- accessor -----------------------------------------------------

	void SetStartOffsetTranslation(const Vector3& startOffsetTranslation);

	const Vector3& GetCurrentOffset() const { return targetOffsetTranslation_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// parameters
	float lerpTimer_; // 補間経過時間
	float lerpTime_;  // 補間時間
	float waitTimer_; // 補間終了後の待ち経過時間
	float waitTime_;  // 補間終了後の待ち時間
	float lerpRate_;  // 補間割合

	Vector3 startOffsetTranslation_;  // 追従相手との距離開始値
	Vector3 targetOffsetTranslation_; // 追従相手との距離目標値
	Vector3 interTarget_;

	Vector3 startRotate_;  // 回転の開始値
	Vector3 targetRotate_; // 回転の目標値

	float startFovY_;  // 画角開始値
	float targetFovY_; // 画角目標値

	EasingType easingType_;
};