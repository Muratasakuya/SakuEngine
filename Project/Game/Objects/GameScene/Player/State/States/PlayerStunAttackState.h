#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Buffer/PostProcessBufferSize.h>
#include <Engine/Object/Base/GameObject3D.h>
#include <Engine/Effect/User/EffectGroup.h>
#include <Engine/Utility/Timer/DelayedHitstop.h>
#include <Engine/Utility/Enum/Easing.h>
#include <Game/Objects/GameScene/Player/State/Interface/PlayerBaseAttackState.h>

//============================================================================
//	PlayerStunAttackState class
//	プレイヤーのスタン攻撃状態(敵がスタンしているとき)
//============================================================================
class PlayerStunAttackState :
	public PlayerBaseAttackState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerStunAttackState();
	~PlayerStunAttackState() = default;

	void Enter(Player& player) override;

	void Update(Player& player) override;
	void UpdateAlways(Player& player) override;

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

	//--------- structure ----------------------------------------------------

	enum class State {

		SubPlayerAttack, // サブプレイヤーの攻撃
		PlayerAttack     // プレイヤーの攻撃
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// プレイヤーの攻撃補間
	float bossEnemyDistance_; // ボスとの距離
	float moveDistance_;      // 移動距離
	Vector3 startPlayerPos_;
	Vector3 targetPlayerPos_;
	StateTimer playerMoveTimer_;

	// ヒットストップ
	bool isHitStopStart_ = false;
	float startHitStopProgress_;
	DelayedHitstop hitStop_;

	// ヒットエフェクト
	std::unique_ptr<EffectGroup> hitEffect_;
	float hitEffectOffsetY_;

	//--------- functions ----------------------------------------------------

	// update
	void UpdateSubPlayerAttack(Player& player);
	void UpdatePlayerAttack(Player& player);
};