#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Player/State/Interface/PlayerBaseAttackState.h>

//============================================================================
//	PlayerAttack_1stState class
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

	//--------- functions ----------------------------------------------------

	void SetActionProgress();
};