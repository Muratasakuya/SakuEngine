#include "PlayerStunAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/SubPlayer/Entity/SubPlayer.h>

//============================================================================
//	PlayerStunAttackState classMethods
//============================================================================

void PlayerStunAttackState::Enter([[maybe_unused]] Player& player) {

	currentState_ = State::SubPlayerAttack;
	canExit_ = false;

	// サブプレイヤーに攻撃させる
	subPlayer_->SetRequestState(SubPlayerState::PunchAttack);

	// プレイヤー一旦消す
	player.SetAlpha(0.0f);

	// サブプレイヤー攻撃用のカメラアニメーション開始
	followCamera_->StartPlayerActionAnim("subPlayerAttack");
}

void PlayerStunAttackState::Update(Player& player) {

	// 状態に応じて更新
	switch (currentState_) {
	case PlayerStunAttackState::State::SubPlayerAttack: {

		UpdateSubPlayerAttack(player);
		break;
	}
	case PlayerStunAttackState::State::PlayerAttack: {

		UpdatePlayerAttack(player);
		break;
	}
	}
}

void PlayerStunAttackState::UpdateSubPlayerAttack(Player& player) {

	// サブプレイヤーの攻撃が終了したかチェック
	if (subPlayer_->IsFinishPunchAttack()) {

		// プレイヤーの攻撃に移行
		player.SetAlpha(1.0f);

		// 次の状態に遷移させる
		currentState_ = State::PlayerAttack;

		// プレイヤー攻撃用のカメラアニメーション開始
		followCamera_->EndPlayerActionAnim(false);
		followCamera_->StartPlayerActionAnim(PlayerState::StunAttack);
	}
}

void PlayerStunAttackState::UpdatePlayerAttack(Player& player) {

	// animationが終了次第遷移可能状態にする
	if (player.IsAnimationFinished()) {

		canExit_ = true;
		// カメラアニメーション終了
		followCamera_->EndPlayerActionAnim(true);
	}
}

void PlayerStunAttackState::Exit([[maybe_unused]] Player& player) {

	// リセット
	canExit_ = false;
}

void PlayerStunAttackState::ImGui([[maybe_unused]] const Player& player) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.01f);
}

void PlayerStunAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
}

void PlayerStunAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
}