#include "SelectUIGamePadInput.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>

//============================================================================
//	SelectUIGamePadInput classMethods
//============================================================================

float SelectUIGamePadInput::GetVector(SelectUIInputAction axis) const {

	// 左スティック入力
	const Vector2 value = input_->GetLeftStickVal();
	switch (axis) {
	case SelectUIInputAction::Up:
	case SelectUIInputAction::Down: {

		return value.y;
	}
	case SelectUIInputAction::Left:
	case SelectUIInputAction::Right: {

		return value.x;
	}
	}
	return 0.0f;
}

bool SelectUIGamePadInput::IsPressed(SelectUIInputAction button) const {

	switch (button) {
	case SelectUIInputAction::Up: {

		return input_->PushGamepadButton(GamePadButtons::ARROW_UP);
	}
	case SelectUIInputAction::Down: {

		return input_->PushGamepadButton(GamePadButtons::ARROW_DOWN);
	}
	case SelectUIInputAction::Left: {

		return input_->PushGamepadButton(GamePadButtons::ARROW_LEFT);
	}
	case SelectUIInputAction::Right: {

		return input_->PushGamepadButton(GamePadButtons::ARROW_RIGHT);
	}
	case SelectUIInputAction::Decide: {

		return input_->PushGamepadButton(GamePadButtons::A);
	}
	case SelectUIInputAction::Return: {

		return input_->PushGamepadButton(GamePadButtons::B);
	}
	}
	return false;
}

bool SelectUIGamePadInput::IsTriggered(SelectUIInputAction button) const {

	switch (button) {
	case SelectUIInputAction::Up: {

		return input_->TriggerGamepadButton(GamePadButtons::ARROW_UP);
	}
	case SelectUIInputAction::Down: {

		return input_->TriggerGamepadButton(GamePadButtons::ARROW_DOWN);
	}
	case SelectUIInputAction::Left: {

		return input_->TriggerGamepadButton(GamePadButtons::ARROW_LEFT);
	}
	case SelectUIInputAction::Right: {

		return input_->TriggerGamepadButton(GamePadButtons::ARROW_RIGHT);
	}
	case SelectUIInputAction::Decide: {

		return input_->TriggerGamepadButton(GamePadButtons::A);
	}
	case SelectUIInputAction::Return: {

		return input_->TriggerGamepadButton(GamePadButtons::B);
	}
	}
	return false;
}