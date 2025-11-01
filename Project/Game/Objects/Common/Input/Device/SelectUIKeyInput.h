#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/IInputDevice.h>
#include <Game/Objects/Common/Input/SelectUIInputAction.h>

//============================================================================
//	SelectUIKeyInput class
//	UI選択用キーボード入力
//============================================================================
class SelectUIKeyInput :
	public IInputDevice<SelectUIInputAction> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SelectUIKeyInput(Input* input) { input_ = input; }
	~SelectUIKeyInput() = default;

	//--------- accessor -----------------------------------------------------

	// 連続入力
	float GetVector(SelectUIInputAction axis)  const override;

	// 単入力
	bool IsPressed(SelectUIInputAction button) const override;
	bool IsTriggered(SelectUIInputAction button) const override;
};