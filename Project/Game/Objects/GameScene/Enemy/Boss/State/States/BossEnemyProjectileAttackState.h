#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/EffectGroup.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/Interface/BossEnemyIState.h>

//============================================================================
//	BossEnemyProjectileAttackState class
//============================================================================
class BossEnemyProjectileAttackState :
	public BossEnemyIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyProjectileAttackState(uint32_t phaseCount);
	~BossEnemyProjectileAttackState() = default;

	void Enter(BossEnemy& bossEnemy) override;

	void Update(BossEnemy& bossEnemy) override;
	void UpdateAlways(BossEnemy& bossEnemy) override;

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

	// 状態
	enum class State {

		Pre,    // 予備動作
		Launch, // 発生装置起動
		Attack, // 順に攻撃
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// フェーズに応じた弾を飛ばす数、奇数個
	std::vector<uint32_t> phaseBulletCounts_;

	// 発生しきる時間、0.0f~1.0fの間に等間隔に発生
	StateTimer launchTimer_;
	// 攻撃が終了するまでの時間、弾の数に応じて変える
	// 等間隔で弾を発生させる
	float bulletAttackDuration_;
	StateTimer attackTimer_;

	// 真ん中の発生位置のY座標
	float launchTopPosY_;
	// 左右の発生位置のオフセット
	Vector3 launchOffsetPos_;

	// 実際に発生させる位置
	std::vector<Vector3> launchPositions_;

	// 準備エフェクト
	std::unique_ptr<EffectGroup> preEffect_;
	// 発生起動エフェクト
	std::unique_ptr<EffectGroup> launchEffect_;
	// 弾
	std::unique_ptr<EffectGroup> bulletEffect_;

	// エディター
	int32_t editingPhase_; // 編集中のフェーズ

	//--------- functions ----------------------------------------------------

	// update
	void UpdatePre(BossEnemy& bossEnemy);
	void UpdateLaunch(BossEnemy& bossEnemy);
	void UpdateAttack(BossEnemy& bossEnemy);

	// helper
	// 発生位置を設定
	void SetLaunchPositions(const BossEnemy& bossEnemy, int32_t phaseIndex);
};