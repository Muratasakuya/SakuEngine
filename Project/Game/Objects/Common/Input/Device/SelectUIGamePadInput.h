#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/IInputDevice.h>
#include <Game/Objects/Common/Input/SelectUIInputAction.h>

//============================================================================
//	SelectUIGamePadInput class
//	UI選択用ゲームパッド入力
//============================================================================
class SelectUIGamePadInput :
	public IInputDevice<SelectUIInputAction> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SelectUIGamePadInput(Input* input) { input_ = input; }
	~SelectUIGamePadInput() = default;

	//--------- accessor -----------------------------------------------------

	// 連続入力
	float GetVector(SelectUIInputAction axis)  const override;

	// 単入力
	bool IsPressed(SelectUIInputAction button) const override;
	bool IsTriggered(SelectUIInputAction button) const override;
};