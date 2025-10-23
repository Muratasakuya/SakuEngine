#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Enemy/Boss/State/Interface/BossEnemyIState.h>
#include <Game/Objects/GameScene/Enemy/Boss/Collision/BossEnemyBladeCollision.h>
#include <Game/Objects/GameScene/Enemy/Boss/Effect/BossEnemyBladeEffect.h>

// c++
#include <array>

//============================================================================
//	BossEnemyRushAttackState class
//	連続刃発生攻撃状態
//============================================================================
class BossEnemyRushAttackState :
	public BossEnemyIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyRushAttackState();
	~BossEnemyRushAttackState() = default;

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

	// 現在の状態
	enum class State {

		Teleport, // テレポート
		Attack,   // 攻撃
		Cooldown  // 攻撃不可能状態
	};

	// 攻撃の種類
	struct AttackPattern {

		BossEnemyTeleportType teleportType;
		std::string animationName;
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// 攻撃のパターン
	std::array<AttackPattern, 3> pattern_ = {
		AttackPattern{ BossEnemyTeleportType::Far, "bossEnemy_rushAttack" },
		AttackPattern{ BossEnemyTeleportType::Far, "bossEnemy_rushAttack" },
		AttackPattern{ BossEnemyTeleportType::Far, "bossEnemy_chargeAttack" }
	};

	// parameters
	float farRadius_;  // 扇形半径(遠くに移動)
	float nearRadius_; // 扇形半径(近くに移動)
	float halfAngle_;  // 扇形の半開き角
	float moveClampSize_; // 移動範囲制限

	// 座標
	Vector3 startPos_;  // 開始座標
	Vector3 targetPos_; // 目標座標

	float lerpTimer_;       // 座標補間の際の経過時間
	float lerpTime_;        // 座標補間にかける時間
	EasingType easingType_; // 補間の際のイージング

	float attackCoolTimer_; // 攻撃クールタイムの経過時間
	float attackCoolTime_;  // 攻撃クールタイム

	int maxAttackCount_;     // 攻撃回数
	int currentAttackCount_; // 現在の攻撃回数

	float fadeOutTime_;  // テレポート開始時の時間
	float fadeInTime_;   // テレポート終了時の時間
	float currentAlpha_; // α値

	float emitParticleOffsetY_; // particleの発生位置のオフセット

	// 3本の刃
	static const uint32_t bladeMaxCount_ = 3;
	std::array<std::unique_ptr<BossEnemyBladeCollision>, bladeMaxCount_> divisionBlades_;
	float divisionBladeMoveSpeed_; // 刃の進む速度
	float divisionOffsetAngle_;    // 刃の角度(0: -angle,1: 0.0f,2: +angle)

	// 1本の刃
	std::unique_ptr<BossEnemyBladeCollision> singleBlade_;
	float singleBladeMoveSpeed_; // 刃の進む速度
	// エフェクト
	// エフェクト、エンジン機能変更中...
	//std::unique_ptr<BossEnemySingleBladeEffect> singleBladeEffect_;
	float singleBladeEffectScalingValue_;

	//--------- functions ----------------------------------------------------

	// init
	void InitBlade();

	// 各状態の更新
	void UpdateTeleport(BossEnemy& bossEnemy, float deltaTime);
	void UpdateAttack(BossEnemy& bossEnemy);
	void UpdateCooldown(BossEnemy& bossEnemy, float deltaTime);

	void UpdateBlade(BossEnemy& bossEnemy);

	// helper
	Vector3 CalcBaseDir(const BossEnemy& bossEnemy) const;
	Vector3 CalcDivisionBladeDir(const BossEnemy& bossEnemy, uint32_t idx) const;
	void EmitDivisionBlades(const BossEnemy& bossEnemy);
	void EmitSingleBlade(const BossEnemy& bossEnemy);
};