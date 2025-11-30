#include "SubPlayerInactiveState.h"

//============================================================================
//	SubPlayerInactiveState classMethods
//============================================================================

void SubPlayerInactiveState::Enter() {

}

void SubPlayerInactiveState::Update() {

	// 非アクティブ状態に移行
	body_->SetScale(Vector3::AnyInit(0.0f));
	rightHand_->SetScale(Vector3::AnyInit(0.0f));
	leftHand_->SetScale(Vector3::AnyInit(0.0f));

	// 状態終了フラグを立てる
	canExit_ = true;
}

void SubPlayerInactiveState::Exit() {

}

void SubPlayerInactiveState::ImGui() {

}

void SubPlayerInactiveState::ApplyJson([[maybe_unused]] const Json& data) {

}

void SubPlayerInactiveState::SaveJson([[maybe_unused]] Json& data) {

}