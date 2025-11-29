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
	player.GetWeapon(PlayerWeaponType::Right)->SetAlpha(0.0f);
	player.GetWeapon(PlayerWeaponType::Left)->SetAlpha(0.0f);

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

		// 次の状態に遷移させる
		currentState_ = State::PlayerAttack;

		// プレイヤーの攻撃に移行
		player.SetAlpha(1.0f);
		player.GetWeapon(PlayerWeaponType::Right)->SetAlpha(1.0f);
		player.GetWeapon(PlayerWeaponType::Left)->SetAlpha(1.0f);

		// アニメーション開始
		player.SetNextAnimation("player_stunAttack", false, nextAnimDuration_);

		// 敵の前方ベクトル方向
		Vector3 bossPos = bossEnemy_->GetTranslation();
		Vector3 bossForward = bossEnemy_->GetTransform().GetForward();
		// プレイヤーの攻撃位置を設定
		// 開始位置、ボスの位置から移動距離分離す
		startPlayerPos_ = bossPos + bossForward * moveDistance_;
		// 目標位置、ボスの位置から少し手前に離す
		targetPlayerPos_ = bossPos + bossForward * bossEnemyDistance_;

		// 最初の位置に設定しておく
		player.SetTranslation(startPlayerPos_);

		// 敵の方に向ける
		// プレイヤーからボス敵への方向を取得
		Vector3 playerPos = player.GetTranslation();
		playerPos.y = 0.0f;
		bossPos.y = 0.0f;
		Vector3 direction = Vector3(bossPos - playerPos).Normalize();
		Quaternion rotation = Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f));
		player.SetRotation(Quaternion::Normalize(rotation));
		player.UpdateMatrix();

		// プレイヤー攻撃用のカメラアニメーション開始
		followCamera_->EndPlayerActionAnim(false);
		followCamera_->StartPlayerActionAnim(PlayerState::StunAttack);
	}
}

void PlayerStunAttackState::UpdatePlayerAttack(Player& player) {

	// アニメーションの切り替えが終わるまで補間しない
	if (player.IsAnimationTransition()) {
		return;
	}

	// 時間更新
	playerMoveTimer_.Update();

	// プレイヤーの位置補間
	Vector3 lerpPos = Vector3::Lerp(startPlayerPos_, targetPlayerPos_, playerMoveTimer_.easedT_);

	// 位置を設定
	player.SetTranslation(lerpPos);

	// 時間経過で状態終了
	if (playerMoveTimer_.IsReached()) {

		canExit_ = true;
	}
}

void PlayerStunAttackState::Exit([[maybe_unused]] Player& player) {

	// リセット
	canExit_ = false;
	playerMoveTimer_.Reset();

	// カメラアニメーション終了
	followCamera_->EndPlayerActionAnim(true);
	followCamera_->SetState(FollowCameraState::Follow);
}

void PlayerStunAttackState::ImGui([[maybe_unused]] const Player& player) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.01f);
	ImGui::DragFloat("bossEnemyDistance", &bossEnemyDistance_, 0.01f);
	ImGui::DragFloat("moveDistance", &moveDistance_, 0.01f);

	playerMoveTimer_.ImGui("playerMoveTimer");
}

void PlayerStunAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	bossEnemyDistance_ = data.value("bossEnemyDistance_", 1.0f);
	moveDistance_ = data.value("moveDistance_", 1.0f);

	playerMoveTimer_.FromJson(data.value("playerMoveTimer_", Json()));
}

void PlayerStunAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["bossEnemyDistance_"] = bossEnemyDistance_;
	data["moveDistance_"] = moveDistance_;

	playerMoveTimer_.ToJson(data["playerMoveTimer_"]);
}