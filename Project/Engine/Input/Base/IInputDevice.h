#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <typeinfo>
// front
class Input;

// enumのみに制約をかける
template<typename T>
concept InputEnum = std::is_enum_v<T>;

//============================================================================
//	IInputDevice class
//	入力デバイスの基底インターフェース、Enumのみをテンプレート引数に取る
//============================================================================
template<InputEnum Enum>
class IInputDevice {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IInputDevice() = default;
	virtual ~IInputDevice() = default;

	//--------- accessor -----------------------------------------------------

	// 連続入力
	virtual float GetVector(Enum axis) const = 0;

	// 単入力
	virtual bool IsPressed(Enum button) const = 0;
	virtual bool IsTriggered(Enum button) const = 0;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Input* input_;
};