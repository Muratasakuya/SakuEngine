#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Camera/Follow/State/Interface/FollowCameraIState.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	FollowCameraAllyAttackState class
//============================================================================
class FollowCameraAllyAttackState :
	public FollowCameraIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraAllyAttackState(float targetFovY);
	~FollowCameraAllyAttackState() = default;

	void Enter(FollowCamera& followCamera) override;

	void Update(FollowCamera& followCamera) override;

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
	Vector3 offsetTranslation_; // 追従相手との距離
	Vector3 interTarget_;       // 追従中間target位置
	float lerpRate_;            // 補間割合

	float fovYLerpTimer_; // 画角遷移経過時間
	float fovYLerpTime_;  // 画角遷移時間
	EasingType fovYLerpEasingType_;

	std::optional<float> startFovY_; // 開始画角
	float targetFovY_;               // 目標画角
};