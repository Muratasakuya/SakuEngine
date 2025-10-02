#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Player/State/Interface/PlayerBaseAttackState.h>

// c++
#include <array>

//============================================================================
//	PlayerAttack_2ndState class
//============================================================================
class PlayerAttack_2ndState :
	public PlayerBaseAttackState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerAttack_2ndState(Player* player);
	~PlayerAttack_2ndState() = default;

	void Enter(Player& player) override;

	void Update(Player& player) override;

	void Exit(Player& player) override;

	// imgui
	void ImGui(const Player& player) override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;

	//--------- accessor -----------------------------------------------------

	bool GetCanExit() const override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	static constexpr size_t kNumSegments = 3;

	// 敵との距離に応じた処理を行うフラグ
	bool approachPhase_;
	bool loopApproach_ = false;

	// parameters
	Vector3 startTranslation_;                    // 移動開始地点
	std::array<Vector3, kNumSegments> wayPoints_; // 移動する点の数
	size_t currentIndex_; // 現在の区間
	float segmentTimer_;  // 区間経過時間
	float segmentTime_;   //区間にかかる時間
	float swayRate_;      //揺れ幅

	float leftPointAngle_;  // 左の座標の角度
	float rightPointAngle_; // 右の座標の角度

	// 座標補間を行わないときの処理
	float approachForwardDistance_; // 前方に進む距離
	float approachSwayLength_;      // 左右振れ幅
	float approachLeftPointAngle_;  // 左の角度
	float approachRightPointAngle_; // 右の角度

	// debug
	std::array<Vector3, kNumSegments> debugWayPoints_;
	std::array<Vector3, kNumSegments> debugApproachWayPoints_;

	//--------- functions ----------------------------------------------------

	// helper
	void CalcWayPoints(const Player& player, std::array<Vector3, kNumSegments>& dstWayPoints);
	void CalcWayPointsToTarget(const Vector3& start, const Vector3& target,
		float leftT, float rightT, float swayLength,
		std::array<Vector3, kNumSegments>& dstWayPoints);
	void CalcApproachWayPoints(const Player& player, std::array<Vector3, kNumSegments>& dstWayPoints);
	bool LerpAlongSegments(Player& player);

	void SetActionProgress();
};