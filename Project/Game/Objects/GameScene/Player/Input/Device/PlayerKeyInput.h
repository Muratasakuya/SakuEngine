#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/IInputDevice.h>
#include <Game/Objects/GameScene/Player/Input/PlayerInputAction.h>

//============================================================================
//	PlayerKeyInput class
//	キーボード入力
//============================================================================
class PlayerKeyInput :
	public IInputDevice<PlayerInputAction> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerKeyInput(Input* input) { input_ = input; }
	~PlayerKeyInput() = default;

	//--------- accessor -----------------------------------------------------

	// 連続入力
	float GetVector(PlayerInputAction axis)  const override;

	// 単入力
	bool IsPressed(PlayerInputAction button) const override;
	bool IsTriggered(PlayerInputAction button) const override;
};