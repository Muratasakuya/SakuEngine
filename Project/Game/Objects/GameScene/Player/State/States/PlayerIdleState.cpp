#include "PlayerIdleState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	PlayerIdleState classMethods
//============================================================================

PlayerIdleState::PlayerIdleState() {
}

void PlayerIdleState::Enter(Player& player) {

	canExit_ = false;
	player.SetNextAnimation("player_idle", true, nextAnimDuration_);
}

void PlayerIdleState::Update([[maybe_unused]] Player& player) {

	canExit_ = true;
}

void PlayerIdleState::UpdateAlways([[maybe_unused]] Player& player) {
}

void PlayerIdleState::Exit([[maybe_unused]] Player& player) {

	canExit_ = false;
}

void PlayerIdleState::ImGui([[maybe_unused]] const Player& player) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
}

void PlayerIdleState::ApplyJson(const Json& data) {

	nextAnimDuration_ = data.value("nextAnimDuration_", 0.01f);
}

void PlayerIdleState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
}