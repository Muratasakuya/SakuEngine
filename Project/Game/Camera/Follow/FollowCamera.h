#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Camera/BaseCamera.h>
#include <Game/Objects/GameScene/Player/Structures/PlayerStructures.h>

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
	void LoadAnim();

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

	void StartPlayerActionAnim(PlayerState state);
	void EndPlayerActionAnim(PlayerState state);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------
	
	// 状態の管理
	std::unique_ptr<FollowCameraStateController> stateController_;

	// アニメーションを読み込んだか
	bool isLoadedAnim_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};