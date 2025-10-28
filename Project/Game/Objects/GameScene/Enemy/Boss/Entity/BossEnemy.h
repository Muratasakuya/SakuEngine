#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>

// scene
#include <Game/Scene/GameState/GameSceneState.h>
// weapon
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemyWeapon.h>
// state
#include <Game/Objects/GameScene/Enemy/Boss/State/BossEnemyStateController.h>
// collision
#include <Game/Objects/GameScene/Enemy/Boss/Collision/BossEnemyAttackCollision.h>
// HUD
#include <Game/Objects/GameScene/Enemy/Boss/HUD/BossEnemyHUD.h>
// effect
#include <Game/Objects/GameScene/Enemy/Boss/Effect/BossEnemyAnimationEffect.h>

// front
class Player;

//============================================================================
//	BossEnemy class
//	ボスクラス
//============================================================================
class BossEnemy :
	public GameObject3D {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemy() = default;
	~BossEnemy() = default;

	// 初期化
	void DerivedInit() override;

	// ボスの状態更新
	void Update(GameSceneState sceneState);

	// エディター
	void DerivedImGui() override;

	/*-------- collision ----------*/

	// 衝突コールバック関数
	void OnCollisionEnter([[maybe_unused]] const CollisionBody* collisionBody) override;

	/*---------- parry ------------*/

	// パリィ受付
	bool ConsumeParryTiming();
	// パリィ受付可能時間の通知
	void TellParryTiming();
	// パリィ受付時間リセット
	void ResetParryTiming() { parryTimingTickets_ = 0; }

	/*---------- falter -----------*/

	// 怯み状態の通知
	void OnFalterState() { ++stats_.currentFalterCount; }
	// 怯みタイマーリセット
	void ResetFalterTimer() { stats_.reFalterTimer.Reset(); }
	// 怯みクールダウン更新
	void UpdateFalterCooldown();

	//--------- accessor -----------------------------------------------------

	void SetPlayer(Player* player);
	void SetFollowCamera(FollowCamera* followCamera);

	void SetAlpha(float alpha);
	void SetCastShadow(bool cast);
	void SetDecreaseToughnessProgress(float progress);

	BossEnemyAttackCollision* GetAttackCollision() const { return attackCollision_.get(); }
	BossEnemyHUD* GetHUD() const { return hudSprites_.get(); }

	Vector3 GetWeaponTranslation() const;
	bool IsCurrentStunState() const;

	int GetDamage() const;
	bool IsDead() const;
	uint32_t GetCurrentPhaseIndex() const;
	const ParryParameter& GetParryParam() const { return stateController_->GetParryParam(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const Player* player_;
	GameSceneState preSceneState_;

	// 使用する武器
	std::unique_ptr<BossEnemyWeapon> weapon_;

	// 状態の管理
	std::unique_ptr<BossEnemyStateController> stateController_;

	// 攻撃の衝突
	std::unique_ptr<BossEnemyAttackCollision>  attackCollision_;

	// HUD
	std::unique_ptr<BossEnemyHUD> hudSprites_;

	// エフェクト、エンジン機能変更中...
	//std::unique_ptr<BossEnemyAnimationEffect> animationEffect_;

	// parameters
	Transform3D initTransform_; // 初期化時の値
	BossEnemyStats stats_;      // ステータス
	uint32_t parryTimingTickets_ = 0;

	// editor
	int selectedPhaseIndex_;
	BossEnemyState editingState_;
	bool isDrawDistanceLevel_ = false;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// init
	void InitWeapon();
	void InitAnimations();
	void InitCollision();
	void InitState();
	void InitHUD();

	// update
	void UpdateBeginGame();
	void UpdatePlayGame();
	void UpdateEndGame();
	void CheckSceneState(GameSceneState sceneState);

	// helper
	void SetInitTransform();
	void CalDistanceToTarget();
	void DebugCommand();
};