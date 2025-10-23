#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Player/State/Interface/PlayerIState.h>

//============================================================================
//	PlayerWalkState class
//	歩き状態
//============================================================================
class PlayerWalkState :
	public PlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerWalkState() = default;
	~PlayerWalkState() = default;

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

	//--------- variables ----------------------------------------------------

	Vector3 move_;    // 移動量
	float moveSpeed_; // 移動速度
	float moveDecay_; // 移動減衰率

	//--------- functions ----------------------------------------------------

	void UpdateWalk(Player& player);
};