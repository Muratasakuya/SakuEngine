#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/EffectGroup.h>
#include <Game/Objects/GameScene/Player/State/Interface/PlayerBaseAttackState.h>

//============================================================================
//	PlayerAttack_1stState class
//	1段目の攻撃
//============================================================================
class PlayerAttack_1stState :
	public PlayerBaseAttackState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerAttack_1stState(Player* player);
	~PlayerAttack_1stState() = default;

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

	//--------- variables ----------------------------------------------------

	bool assisted_;

	// parameters
	// 座標補間を行わないときの処理
	StateTimer moveTimer_;
	float moveValue_;   // 移動量
	Vector3 startPos_;  // 開始座標
	Vector3 targetPos_; // 目標座標

	// 剣エフェクト
	std::unique_ptr<EffectGroup> slashEffect_;
	Vector3 slashEffectOffset_; // 発生位置のオフセット

	//--------- functions ----------------------------------------------------

	void SetActionProgress();
	void SetSpanUpdate(int objectID);
};