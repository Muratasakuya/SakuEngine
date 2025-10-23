#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/IInputDevice.h>
#include <Game/Objects/GameScene/Player/Input/PlayerInputAction.h>

//============================================================================
//	PlayerGamePadInput class
//	ゲームパッド入力
//============================================================================
class PlayerGamePadInput :
	public IInputDevice<PlayerInputAction> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerGamePadInput(Input* input) { input_ = input; }
	~PlayerGamePadInput() = default;

	//--------- accessor -----------------------------------------------------

	// 連続入力
	float GetVector(PlayerInputAction axis)  const override;

	// 単入力
	bool IsPressed(PlayerInputAction button) const override;
	bool IsTriggered(PlayerInputAction button) const override;
};