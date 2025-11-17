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

	GameTimer::StartHitStop(hitStopTime_, 0.0f);

	// カメラの向きを補正させる
	followCamera_->StartLookToTarget(FollowCameraTargetType::Player,
		FollowCameraTargetType::BossEnemy, true, true, targetCameraRotateX_);

	canExit_ = false;
}

void PlayerFalterState::Update(Player& player) {

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
}

void PlayerFalterState::ImGui([[maybe_unused]] const Player& player) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.01f);
	ImGui::DragFloat("moveDistance", &moveDistance_, 0.1f);
	ImGui::DragFloat("targetCameraRotateX", &targetCameraRotateX_, 0.01f);
	ImGui::DragFloat("hitStopTime", &hitStopTime_, 0.01f);

	falterTimer_.ImGui("FalterTimer");
}

void PlayerFalterState::ApplyJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	nextAnimDuration_ = data.value("nextAnimDuration_", 0.1f);
	moveDistance_ = data.value("moveDistance_", 0.5f);
	targetCameraRotateX_ = data.value("targetCameraRotateX_", 0.5f);
	hitStopTime_ = data.value("hitStopTime_", 0.05f);

	falterTimer_.FromJson(data.value("FalterTimer", Json()));
}

void PlayerFalterState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["moveDistance_"] = moveDistance_;
	data["targetCameraRotateX_"] = targetCameraRotateX_;
	data["hitStopTime_"] = hitStopTime_;

	falterTimer_.ToJson(data["FalterTimer"]);
}