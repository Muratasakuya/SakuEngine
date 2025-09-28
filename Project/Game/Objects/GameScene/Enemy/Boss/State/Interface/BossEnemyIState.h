#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Enemy/Boss/Structures/BossEnemyStructures.h>
#include <Engine/Utility/Enum/Easing.h>
#include <Engine/MathLib/MathUtils.h>

// front
class BossEnemy;
class BossEnemyAttackSign;
class Player;
class FollowCamera;

//============================================================================
//	BossEnemyIState class
//============================================================================
class BossEnemyIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyIState() = default;
	virtual ~BossEnemyIState() = default;

	// 状態遷移時
	virtual void Enter(BossEnemy& bossEnemy) = 0;

	// 更新処理
	virtual void Update(BossEnemy& bossEnemy) = 0;
	virtual void UpdateAlways([[maybe_unused]] BossEnemy& bossEnemy) {}

	// 状態終了時
	virtual void Exit(BossEnemy& bossEnemy) = 0;

	// imgui
	virtual void ImGui(const BossEnemy& bossEnemy) = 0;

	// json
	virtual void ApplyJson(const Json& data) = 0;
	virtual void SaveJson(Json& data) = 0;

	//--------- accessor -----------------------------------------------------

	void SetPlayer(const Player* player) { player_ = player; }
	void SetFollowCamera(const FollowCamera* followCamera) { followCamera_ = followCamera; }
	void SetAttackSign(BossEnemyAttackSign* attackSign) { attackSign_ = attackSign; }

	virtual bool GetCanExit() const { return canExit_; }
	const ParryParameter& GetParryParam() const { return parryParam_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const Player* player_;
	const FollowCamera* followCamera_;
	BossEnemyAttackSign* attackSign_;

	// 共通parameters
	float nextAnimDuration_; // 次のアニメーション遷移にかかる時間
	bool canExit_ = true;    // 遷移可能かどうか
	float rotationLerpRate_; // 回転補完割合
	ParryParameter parryParam_;

	//--------- functions ----------------------------------------------------

	// helper
	void LookTarget(BossEnemy& bossEnemy, const Vector3& target);
	Vector2 ProjectToScreen(const Vector3& translation, const FollowCamera& camera);
};