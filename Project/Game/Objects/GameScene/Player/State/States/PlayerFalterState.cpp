#include "PlayerFalterState.h"

//============================================================================
//	include
//============================================================================
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	PlayerFalterState classMethods
//============================================================================

PlayerFalterState::PlayerFalterState(Player* player) {

	player_ = nullptr;
	player_ = player;
}

void PlayerFalterState::Enter(Player& player) {

	// 怯みアニメーションを再生
	player.SetNextAnimation("player_falter", false, nextAnimDuration_);

	// 向き
	Vector3 direction = PlayerIState::GetDirectionToBossEnemy();

	//補間座標を設定
	startPos_ = player.GetTranslation();
	targetPos_ = startPos_ + direction * moveDistance_;

	// 敵の方向を向かせる
	player.SetRotation(Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f)));

	// デルタタイムを一時停止
	GameTimer::SetReturnScaleEnable(false);
	GameTimer::SetTimeScale(0.0f);

	// カメラの向きを補正させる
	followCamera_->StartLookToTarget(FollowCameraTargetType::Player,
		FollowCameraTargetType::BossEnemy, true, true, targetCameraRotateX_);

	canExit_ = false;
}

void PlayerFalterState::Update(Player& player) {

	// デルタタイム停止解除まで待機
	deltaWaitTimer_.Update(std::nullopt, false);
	if (deltaWaitTimer_.IsReached()) {

		// 元に戻させる
		GameTimer::SetReturnScaleEnable(true);
	}

	// 時間を更新
	falterTimer_.Update();
	// 座標を補間
	player.SetTranslation(Vector3::Lerp(startPos_, targetPos_, falterTimer_.easedT_));

	// 補間終了、アニメーション後状態を終了する
	if (falterTimer_.IsReached() && player.IsAnimationFinished()) {

		canExit_ = true;
	}
}

void PlayerFalterState::Exit([[maybe_unused]] Player& player) {

	// リセット
	falterTimer_.Reset();
	deltaWaitTimer_.Reset();
}

void PlayerFalterState::ImGui([[maybe_unused]] const Player& player) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.01f);
	ImGui::DragFloat("moveDistance", &moveDistance_, 0.1f);
	ImGui::DragFloat("targetCameraRotateX", &targetCameraRotateX_, 0.01f);

	falterTimer_.ImGui("FalterTimer");
	deltaWaitTimer_.ImGui("DeltaWait");
}

void PlayerFalterState::ApplyJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	nextAnimDuration_ = data.value("nextAnimDuration_", 0.1f);
	moveDistance_ = data.value("moveDistance_", 0.5f);
	targetCameraRotateX_ = data.value("targetCameraRotateX_", 0.5f);

	falterTimer_.FromJson(data.value("FalterTimer", Json()));
	deltaWaitTimer_.FromJson(data.value("DeltaWait", Json()));
}

void PlayerFalterState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["moveDistance_"] = moveDistance_;
	data["targetCameraRotateX_"] = targetCameraRotateX_;

	falterTimer_.ToJson(data["FalterTimer"]);
	deltaWaitTimer_.ToJson(data["DeltaWait"]);
}