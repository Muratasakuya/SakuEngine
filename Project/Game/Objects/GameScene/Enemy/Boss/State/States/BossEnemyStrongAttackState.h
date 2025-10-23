#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Enemy/Boss/State/Interface/BossEnemyIState.h>

//============================================================================
//	BossEnemyStrongAttackState class
//	強攻撃状態
//============================================================================
class BossEnemyStrongAttackState :
	public BossEnemyIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyStrongAttackState();
	~BossEnemyStrongAttackState() = default;

	void Enter(BossEnemy& bossEnemy) override;

	void Update(BossEnemy& bossEnemy) override;

	void Exit(BossEnemy& bossEnemy) override;

	// imgui
	void ImGui(const BossEnemy& bossEnemy) override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- stricture ----------------------------------------------------

	// 状態
	enum class State {

		ParrySign, // パリィ受付中
		Attack1st, // 攻撃1回目
		Attack2nd  // 攻撃2回目
	};

	//--------- variables ----------------------------------------------------

	// 状態
	State currentState_;

	// 座標
	Vector3 startPos_; // 開始座標

	// parameters
	float lerpTimer_;       // 座標補間の際の経過時間
	EasingType easingType_; // 補間の際のイージング

	float attackOffsetTranslation_; // 座標からのオフセット距離

	float exitTimer_; // 遷移可能にするまでの経過時間
	float exitTime_;  // 遷移可能にするまでの時間

	float attack2ndAnimDuration_; // 2回目のアニメーション補間時間
	float attack2ndLerpTime_;     // 座標補間時間
	bool reachedPlayer_; // 近くまで来たかどうか

	// debug
	std::unordered_map<State, bool> parriedMaps_;

	//--------- functions ----------------------------------------------------

	// update
	void UpdateParrySign(BossEnemy& bossEnemy);
	void UpdateAttack1st(BossEnemy& bossEnemy);
	void UpdateAttack2nd(BossEnemy& bossEnemy);
	void UpdateParryTiming(BossEnemy& bossEnemy);

	// helper
	void LerpTranslation(BossEnemy& bossEnemy);
};