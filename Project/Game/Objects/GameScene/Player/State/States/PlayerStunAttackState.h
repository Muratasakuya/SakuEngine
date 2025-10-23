#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Buffer/PostProcessBufferSize.h>
#include <Engine/Object/Base/GameObject3D.h>
#include <Game/Objects/GameScene/Player/State/Interface/PlayerBaseAttackState.h>
#include <Engine/Utility/Enum/Easing.h>

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

	PlayerStunAttackState(GameObject3D* ally);
	~PlayerStunAttackState() = default;

	void Enter(Player& player) override;

	void Update(Player& player) override;

	void Exit(Player& player) override;

	// imgui
	void ImGui(const Player& player) override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	enum class State {

		AllyEntry,      // 味方登場(同時にプレイヤーは去る)
		AllyRushAttack, // 味方の突進攻撃
		PlayerAttack    // プレイヤーの攻撃
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// 味方
	GameObject3D* ally_; // 突進するだけ
	// 登場(これに合わせてplayerの表示を消す)
	float entryTimer_; // 登場経過時間
	float entryTime_;  // 登場時間
	float targetTranslationY_;   // 目標Y座標
	float enemyDistance_;        // 敵との距離
	EasingType entryEasingType_; // イージングの種類
	// 突進
	float rushTimer_; // 突進経過時間
	float rushTime_;  // 突進時間
	Vector3 rushStartAllyTranslation_;  // 補間開始座標
	Vector3 rushTargetAllyTranslation_; // 目標座標
	EasingType rushEasingType_;         // イージングの種類

	//--------- functions ----------------------------------------------------

	// update
	void UpdateAllyEntry(Player& player);
	void UpdateAllyRushAttack(Player& player);
	void UpdatePlayerAttack(Player& player);
};