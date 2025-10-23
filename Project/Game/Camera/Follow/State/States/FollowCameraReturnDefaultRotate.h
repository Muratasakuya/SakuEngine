#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Camera/Follow/State/Interface/FollowCameraIState.h>
#include <Game/Objects/GameScene/Player/Structures/PlayerStructures.h>
#include <Engine/Utility/Timer/StateTimer.h>

//============================================================================
//	FollowCameraReturnDefaultRotate class
//============================================================================
class FollowCameraReturnDefaultRotate :
	public FollowCameraIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraReturnDefaultRotate();
	~FollowCameraReturnDefaultRotate() = default;

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

	// 補間
	Quaternion startTwistX_;    // 開始時のx軸
	Quaternion targetRotation_; // 目標

	// 時間、プレイヤーの状態に応じて目標時間を変える
	std::unordered_map<PlayerState, float> targetTime_;
	std::optional<float> requestTargetTime_;
	StateTimer lerpTimer_;

	// x軸回転
	float startRotateX_;  // 開始x軸
	float targetRotateX_; // 目標x軸
	float lerpThreshold_; // 補間処理を行う回転

	//--------- functions ----------------------------------------------------

};