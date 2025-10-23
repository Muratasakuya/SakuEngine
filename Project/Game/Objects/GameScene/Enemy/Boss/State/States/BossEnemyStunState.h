#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Enemy/Boss/State/Interface/BossEnemyIState.h>

//============================================================================
//	BossEnemyStunState class
//	スタン中の更新処理
//============================================================================
class BossEnemyStunState :
	public BossEnemyIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyStunState(const BossEnemy& bossEnemy);
	~BossEnemyStunState() = default;

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

	//--------- structure ----------------------------------------------------

	enum class State {

		Begin,  // スタン開始
		Update, // スタン中
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	float beginStunAnimationTime_;  // 怯む時のanimation再生時間
	float updateStunAnimationTime_; // 更新中のanimation再生時間
	float toughnessDecreaseTimer_; // 靭性地減少の経過時間
	float toughnessDecreaseTime_;  // 靭性値減少の時間

	// parameters
	int maxAnimationCount_;
};