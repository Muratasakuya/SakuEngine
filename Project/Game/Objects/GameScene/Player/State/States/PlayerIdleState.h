#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Player/State/Interface/PlayerIState.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	PlayerIdleState class
//	何も入力していないときのアイドル状態、基本状態
//============================================================================
class PlayerIdleState :
	public PlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerIdleState();
	~PlayerIdleState() = default;

	void Enter(Player& player) override;

	void Update(Player& player) override;
	void UpdateAlways(Player& player) override;

	void Exit(Player& player) override;

	// imgui
	void ImGui(const Player& player) override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;
};