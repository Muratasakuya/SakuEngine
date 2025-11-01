#include "SelectUIKeyInput.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>

//============================================================================
//	SelectUIKeyInput classMethods
//============================================================================

float SelectUIKeyInput::GetVector([[maybe_unused]] SelectUIInputAction axis) const {
	return 0.0f;
}

bool SelectUIKeyInput::IsPressed(SelectUIInputAction button) const {

	switch (button) {
	case SelectUIInputAction::Up: {

		return input_->PushKey(DIK_W) || input_->PushKey(DIK_UP);
	}
	case SelectUIInputAction::Down: {

		return input_->PushKey(DIK_S) || input_->PushKey(DIK_DOWN);
	}
	case SelectUIInputAction::Left: {

		return input_->PushKey(DIK_A) || input_->PushKey(DIK_LEFT);
	}
	case SelectUIInputAction::Right: {

		return input_->PushKey(DIK_D) || input_->PushKey(DIK_RIGHT);
	}
	case SelectUIInputAction::Decide: {

		return input_->PushKey(DIK_SPACE);
	}
	case SelectUIInputAction::Return: {

		return input_->PushKey(DIK_ESCAPE);
	}
	}
	return false;
}

bool SelectUIKeyInput::IsTriggered(SelectUIInputAction button) const {

	switch (button) {
	case SelectUIInputAction::Up: {

		return input_->TriggerKey(DIK_W) || input_->TriggerKey(DIK_UP);
	}
	case SelectUIInputAction::Down: {

		return input_->TriggerKey(DIK_S) || input_->TriggerKey(DIK_DOWN);
	}
	case SelectUIInputAction::Left: {

		return input_->TriggerKey(DIK_A) || input_->TriggerKey(DIK_LEFT);
	}
	case SelectUIInputAction::Right: {

		return input_->TriggerKey(DIK_D) || input_->TriggerKey(DIK_RIGHT);
	}
	case SelectUIInputAction::Decide: {

		return input_->TriggerKey(DIK_SPACE);
	}
	case SelectUIInputAction::Return: {

		return input_->TriggerKey(DIK_ESCAPE);
	}
	}
	return false;
}