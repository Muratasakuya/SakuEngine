#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Camera/BaseCamera.h>

// state
#include <Game/Camera/Follow/State/FollowCameraStateController.h>

//============================================================================
//	FollowCamera class
//============================================================================
class FollowCamera :
	public BaseCamera {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCamera() = default;
	~FollowCamera() = default;

	void Init() override;

	void Update() override;

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	void SetScreenShake(bool isShake);
	void SetParry(bool isParry);
	void SetParryAttack(bool isParry);
	void SetTarget(FollowCameraTargetType type, const Transform3D& target);
	void SetFovY(float fovY) { fovY_ = fovY; }
	void SetState(FollowCameraState state);

	float GetFovY() const { return fovY_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------
	
	// 状態の管理
	std::unique_ptr<FollowCameraStateController> stateController_;

	//--------- functions ----------------------------------------------------

	// init
	void LoadAnimCamera();

	// json
	void ApplyJson();
	void SaveJson();
};