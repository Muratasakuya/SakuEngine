#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Buffer/PostProcessBufferSize.h>
#include <Engine/Utility/Enum/Easing.h>
#include <Engine/Utility/Timer/StateTimer.h>
#include <Game/Objects/GameScene/Player/State/Interface/PlayerIState.h>
#include <Game/Objects/GameScene/Player/Structure/PlayerStructures.h>

//============================================================================
//	PlayerSwitchAllyState class
//	味方入れ替え状態
//============================================================================
class PlayerSwitchAllyState :
	public PlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerSwitchAllyState();
	~PlayerSwitchAllyState() = default;

	void Enter(Player& player) override;

	void Update(Player& player) override;

	void Exit(Player& player) override;

	// imgui
	void ImGui(const Player& player) override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;

	//--------- accessor -----------------------------------------------------

	PlayerState GetSelectState() const { return selectState_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// デルタタイムのスケール補間時間
	StateTimer deltaTimeScaleTimer_;
	float deltaTimeScale_ = 1.0f; // 目標スケール

	float switchAllyTimer_; // 現在の経過時間
	float switchAllyTime_;  // 切り替え選択の行える時間

	// 切り替えたかどうかのフラグ
	PlayerState selectState_;

	//--------- functions----------------------------------------------------

	// update
	void CheckInput(float t);
};