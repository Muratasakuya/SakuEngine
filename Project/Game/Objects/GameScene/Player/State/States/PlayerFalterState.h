#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Player/State/Interface/PlayerIState.h>
#include <Engine/Utility/Timer/StateTimer.h>

//============================================================================
//	PlayerFalterState class
//	敵の攻撃を受けた時の怯み状態
//============================================================================
class PlayerFalterState :
	public PlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerFalterState(Player* player);
	~PlayerFalterState() = default;

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

	// 怯みの移動補間時間
	StateTimer falterTimer_;
	// 補間座標
	Vector3 startPos_;
	Vector3 targetPos_;
	// 移動距離
	float moveDistance_;

	// デルタタイムの停止時間
	float hitStopTime_;

	//--------- functions ----------------------------------------------------

};