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

	void SetPlayer(const Player* player) { stateController_->SetPlayer(player); }
	void SetTarget(FollowCameraTargetType type, const Transform3D& target);
	void SetFovY(float fovY) { fovY_ = fovY; }
	void SetState(FollowCameraState state);
	void SetOverlayState(FollowCameraOverlayState state, bool isStart);

	float GetFovY() const { return fovY_; }

	// エディターによるカメラアニメーション
	void StartPlayerActionAnim(PlayerState state);
	void EndPlayerActionAnim(PlayerState  state, bool isWarmStart);

	// 視点を注視点に向ける
	void StartLookToTarget(FollowCameraTargetType from, FollowCameraTargetType to,
		bool isReset = false, bool isLockTarget = false,
		std::optional<float> targetXRotation = std::nullopt, float lookTimerRate = 1.0f);
	void SetLookAlwaysTarget(bool look) { lookAlwaysTarget_ = look; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 状態の管理
	std::unique_ptr<FollowCameraStateController> stateController_;
	std::unordered_map<FollowCameraTargetType, const Transform3D*> targets_;

	// 視点を注視点に向ける処理
	bool lookStart_ = false;         // 補間開始するか
	bool  lookAlwaysTarget_ = false; // trueの間ずっと向ける
	float lookTargetLerpRate_; // フレーム補間割合
	std::pair<FollowCameraTargetType, FollowCameraTargetType> lookPair_;
	Quaternion lookToStart_;  // 補間開始時の回転
	std::optional<Quaternion> lookToTarget_; // 補間目標の回転
	StateTimer lookTimer_;   // 補間までの時間
	float lookTimerRate_;    // 目標時間の倍速率
	float targetXRotation_;  // 目標X回転
	std::optional<float> anyTargetXRotation_;
	float startFovY_; // 開始時
	float initFovY_;  // 目標

	float lookYawOffset_;     // Y回転オフセット
	int lookYawDirection_;    // 開始時の最短補間方向
	int preLookYawDirection_; // 開始時の最短補間方向の前回値

	// アニメーションを読み込んだか
	bool isLoadedAnim_;

	//--------- functions ----------------------------------------------------

	// update
	void UpdateLookToTarget();
	void UpdateLookAlwaysTarget();

	// helper
	Quaternion GetTargetRotation() const;

	// json
	void ApplyJson();
	void SaveJson();
};