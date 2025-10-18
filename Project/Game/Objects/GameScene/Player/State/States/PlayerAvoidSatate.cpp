#include "PlayerAvoidSatate.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	PlayerAvoidSatate classMethods
//============================================================================

void PlayerAvoidSatate::Enter(Player& player) {

	player.SetNextAnimation("player_avoid", false, nextAnimDuration_);

	const Vector3 playerPos = player.GetTranslation();
	const Vector3 enemyPos = bossEnemy_->GetTranslation();
	// 向き
	Vector3 direction = (enemyPos - playerPos).Normalize();
	direction.y = 0.0f;
	direction = direction.Normalize();

	// 補間座標を設定する
	startPos_ = playerPos;
	targetPos_ = playerPos + direction * moveDistance_;

	// 敵の方向を向かせる
	player.SetRotation(Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f)));

	canExit_ = false;
}

void PlayerAvoidSatate::Update(Player& player) {

	// 時間を進める
	lerpTimer_ += GameTimer::GetScaledDeltaTime();
	float lerpT = lerpTimer_ / lerpTime_;
	lerpT = EasedValue(easingType_, lerpT);

	// 座標を補間
	Vector3 translation = Vector3::Lerp(startPos_, targetPos_, lerpT);

	// 座標を設定
	player.SetTranslation(translation);

	// 時間が経過しきったら遷移可能状態にする
	if (lerpTime_ < lerpTimer_) {

		canExit_ = true;
	}
}

void PlayerAvoidSatate::Exit([[maybe_unused]] Player& player) {

	// リセット
	lerpTimer_ = 0.0f;
	canExit_ = false;
}

void PlayerAvoidSatate::ImGui(const Player& player) {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.01f);
	ImGui::DragFloat("lerpTime", &lerpTime_, 0.01f);
	ImGui::DragFloat("moveDistance", &moveDistance_, 0.1f);
	Easing::SelectEasingType(easingType_);

	const Vector3 playerPos = player.GetTranslation();
	const Vector3 enemyPos = bossEnemy_->GetTranslation();
	// 向き
	Vector3 direction = (enemyPos - playerPos).Normalize();

	// 補間座標を設定する
	Vector3 startPos = playerPos;
	Vector3 targetPos = playerPos + direction * moveDistance_;
	startPos.y = 4.0f;
	targetPos.y = 4.0f;

	LineRenderer::GetInstance()->DrawLine3D(
		startPos, targetPos, Color::Red());
}

void PlayerAvoidSatate::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	lerpTime_ = JsonAdapter::GetValue<float>(data, "lerpTime_");
	moveDistance_ = JsonAdapter::GetValue<float>(data, "moveDistance_");
	easingType_ = static_cast<EasingType>(JsonAdapter::GetValue<int>(data, "easingType_"));
}

void PlayerAvoidSatate::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["lerpTime_"] = lerpTime_;
	data["moveDistance_"] = moveDistance_;
	data["easingType_"] = static_cast<int>(easingType_);
}