#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Buffer/PostProcessBufferSize.h>
#include <Game/Objects/GameScene/Player/State/Interface/PlayerIState.h>
#include <Game/Objects/GameScene/Player/Structures/PlayerStructures.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	PlayerSwitchAllyState class
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

	float deltaTimeScaleTimer_; // スローモーションにするまでの経過時間
	float deltaTimeScaleTime_;  // スローモーションにするまでの時間
	EasingType deltaTimeScaleEasingType_;
	float deltaTimeScale_;      // スローモーション用

	float switchAllyTimer_; // 現在の経過時間
	float switchAllyTime_;  // 切り替え選択の行える時間

	// 切り替えたかどうかのフラグ
	PlayerState selectState_;

	//--------- functions----------------------------------------------------

	// update
	void CheckInput(float t);
};