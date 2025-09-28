#include "PlayerWalkState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Camera/Follow/FollowCamera.h>

//============================================================================
//	PlayerWalkState classMethods
//============================================================================

void PlayerWalkState::Enter(Player& player) {

	player.SetNextAnimation("player_walk", true, nextAnimDuration_);
}

void PlayerWalkState::Update(Player& player) {

	// 歩き更新
	UpdateWalk(player);
	// 回転、進行方向に向かせる
	SetRotateToDirection(player, move_);
}

void PlayerWalkState::UpdateWalk(Player& player) {

	// 入力値取得
	Vector2 inputValue(inputMapper_->GetVector(PlayerInputAction::MoveX),
		inputMapper_->GetVector(PlayerInputAction::MoveZ));
	inputValue = Vector2::Normalize(inputValue);

	if (std::fabs(inputValue.x) > epsilon_ || std::fabs(inputValue.y) > epsilon_) {

		// 入力がある場合のみ速度を計算する
		Vector3 inputDirection(inputValue.x, 0.0f, inputValue.y);
		inputDirection = Vector3::Normalize(inputDirection);

		Matrix4x4 rotateMatrix = Quaternion::MakeRotateMatrix(followCamera_->GetTransform().rotation);
		Vector3 rotatedDirection = Vector3::TransferNormal(inputDirection, rotateMatrix);
		rotatedDirection = Vector3::Normalize(rotatedDirection);

		move_ = rotatedDirection * moveSpeed_;
	} else {

		// 入力がなければどんどん減速させる
		move_ *= moveDecay_;
		// 一定の速度以下で止まる
		if (move_.Length() < epsilon_) {
			move_.Init();
		}
	}
	move_.y = 0.0f;

	// 移動量を加算
	Vector3 translation = player.GetTranslation();
	translation += move_;
	player.SetTranslation(translation);
}

void PlayerWalkState::Exit([[maybe_unused]] Player& player) {
}

void PlayerWalkState::ImGui([[maybe_unused]] const Player& player) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("moveSpeed", &moveSpeed_, 0.01f);
	ImGui::DragFloat("moveDecay", &moveDecay_, 0.01f);
}

void PlayerWalkState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	moveSpeed_ = JsonAdapter::GetValue<float>(data, "moveSpeed_");
	moveDecay_ = JsonAdapter::GetValue<float>(data, "moveDecay_");
}

void PlayerWalkState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["moveSpeed_"] = moveSpeed_;
	data["moveDecay_"] = moveDecay_;
}