#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/EffectGroup.h>
#include <Game/Objects/GameScene/Player/State/Interface/PlayerBaseAttackState.h>

//============================================================================
//	PlayerAttack_4thState class
//	4段目の攻撃
//============================================================================
class PlayerAttack_4thState :
	public PlayerBaseAttackState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerAttack_4thState(Player* player);
	~PlayerAttack_4thState() = default;

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

	// 地割れエフェクト
	std::unique_ptr<EffectGroup> groundCrackEffect_;
	bool groundCrackEmitted_ = false;

	//--------- functions ----------------------------------------------------

	void SetActionProgress();
	void SetSpanUpdate(int objectID);
};