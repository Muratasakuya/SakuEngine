#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>

// weapon
#include <Game/Objects/GameScene/Player/Entity/PlayerWeapon.h>
// state
#include <Game/Objects/GameScene/Player/State/PlayerStateController.h>
// collision
#include <Game/Objects/GameScene/Player/Collision/PlayerAttackCollision.h>
// HUD
#include <Game/Objects/GameScene/Player/HUD/PlayerHUD.h>
#include <Game/Objects/GameScene/Player/HUD/PlayerStunHUD.h>
// effect
#include <Game/Objects/GameScene/Player/Effect/PlayerAnimationEffect.h>

//============================================================================
//	Player class
//============================================================================
class Player :
	public GameObject3D {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Player() = default;
	~Player() = default;

	void DerivedInit() override;

	void Update() override;

	void DerivedImGui() override;

	/*-------- collision ----------*/

	// 衝突コールバック関数
	void OnCollisionEnter(const CollisionBody* collisionBody) override;

	//--------- accessor -----------------------------------------------------

	void SetBossEnemy(const BossEnemy* bossEnemy);
	void SetFollowCamera(FollowCamera* followCamera);
	void SetReverseWeapon(bool isReverse, PlayerWeaponType type);
	void ResetWeaponTransform(PlayerWeaponType type);

	PlayerState GetCurrentState() const { return stateController_->GetCurrentState(); }

	PlayerAttackCollision* GetAttackCollision() const { return attackCollision_.get(); }
	GameObject3D* GetAlly() const { return ally_.get(); }
	PlayerHUD* GetHUD() const { return hudSprites_.get(); }
	PlayerStunHUD* GetStunHUD() const { return stunHudSprites_.get(); }
	PlayerWeapon* GetWeapon(PlayerWeaponType type) const;

	int GetDamage() const;
	int GetToughness() const { return stats_.toughness; }
	bool IsDead() const { return stats_.currentHP == 0; }
	bool IsActiveParry() const { return stateController_->IsActiveParry(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const BossEnemy* bossEnemy_;

	// 使用する武器
	std::unique_ptr<PlayerWeapon> rightWeapon_; // 右手
	std::unique_ptr<PlayerWeapon> leftWeapon_;  // 左手
	// 味方
	std::unique_ptr<GameObject3D> ally_;

	// 状態の管理
	std::unique_ptr<PlayerStateController> stateController_;

	// 攻撃の衝突
	std::unique_ptr<PlayerAttackCollision> attackCollision_;

	// HUD
	std::unique_ptr<PlayerHUD> hudSprites_;
	std::unique_ptr<PlayerStunHUD> stunHudSprites_;

	// エフェクト、エンジン機能変更中...
	//std::unique_ptr<PlayerAnimationEffect> animationEffect_;

	// parameters
	Transform3D initTransform_; // 初期化時の値
	PlayerStats stats_; // ステータス

	// 敵のスタン中の更新になったか
	bool isStunUpdate_;

	// json
	Json cacheJsonData_;

	// editor
	PlayerState editingState_;

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

	// helper
	void SetInitTransform();
	void CheckBossEnemyStun();
};