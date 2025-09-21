#include "BossEnemyTeleportationState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/GameTimer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyTeleportationState classMethods
//============================================================================

void BossEnemyTeleportationState::Enter(BossEnemy& bossEnemy) {

	bossEnemy.SetNextAnimation("bossEnemy_teleport", true, nextAnimDuration_);

	// 座標を設定
	Vector3 center = player_->GetTranslation();
	center.y = 0.0f;
	const Vector3 forward = followCamera_->GetTransform().GetForward();
	startPos_ = bossEnemy.GetTranslation();
	// 弧上の座標を取得
	if (type_ == BossEnemyTeleportType::Far) {

		targetPos_ = Math::RandomPointOnArcInSquare(center,
			followCamera_->GetTransform().GetForward(),
			farRadius_, halfAngle_, Vector3::AnyInit(0.0f), moveClampSize_ / 2.0f);
	} else if (type_ == BossEnemyTeleportType::Near) {

		targetPos_ = Math::RandomPointOnArcInSquare(center,
			followCamera_->GetTransform().GetForward(),
			nearRadius_, halfAngle_, Vector3::AnyInit(0.0f), moveClampSize_ / 2.0f);
	}

	currentAlpha_ = 1.0f;
	bossEnemy.SetAlpha(currentAlpha_);

	canExit_ = false;
}

void BossEnemyTeleportationState::Update(BossEnemy& bossEnemy) {

	lerpTimer_ += GameTimer::GetScaledDeltaTime();
	float lerpT = std::clamp(lerpTimer_ / lerpTime_, 0.0f, 1.0f);
	lerpT = EasedValue(easingType_, lerpT);

	// 座標補間
	bossEnemy.SetTranslation(Vector3::Lerp(startPos_, targetPos_, lerpT));

	// playerの方を向かせる
	Vector3 playerPos = player_->GetTranslation();
	playerPos.y = 0.0f;
	LookTarget(bossEnemy, playerPos);

	const float disappearEnd = fadeOutTime_;           // 消え終わる時間
	const float appearStart = lerpTime_ - fadeInTime_; // 現れ始める時間

	bossEnemy.SetCastShadow(true);
	if (lerpTimer_ <= disappearEnd) {

		const float t = std::clamp(lerpTimer_ / fadeOutTime_, 0.0f, 1.0f);
		currentAlpha_ = 1.0f - t;
	} else if (lerpTimer_ >= appearStart) {

		const float t = std::clamp((lerpTimer_ - appearStart) / fadeInTime_, 0.0f, 1.0f);
		currentAlpha_ = t;
	} else {

		currentAlpha_ = 0.0f;
		bossEnemy.SetCastShadow(false);
	}
	bossEnemy.SetAlpha(currentAlpha_);

	// 時間経過が過ぎたら状態遷移可能にする
	if (lerpTime_ < lerpTimer_) {

		bossEnemy.SetTranslation(targetPos_);
		canExit_ = true;
	} else {

		Vector3 emitPos = bossEnemy.GetTranslation();
		emitPos.y = emitParticleOffsetY_;
	}
}

void BossEnemyTeleportationState::Exit([[maybe_unused]] BossEnemy& bossEnemy) {

	// リセット
	canExit_ = false;
	lerpTimer_ = 0.0f;
	currentAlpha_ = 1.0f;
}

void BossEnemyTeleportationState::ImGui([[maybe_unused]] const BossEnemy& bossEnemy) {

	// テレポートの種類の名前
	const char* teleportNames[] = { "Far","Near" };
	ImGui::Text("currentTeleportType: %s", teleportNames[static_cast<int>(type_)]);

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);

	ImGui::DragFloat("farRadius:Red", &farRadius_, 0.1f);
	ImGui::DragFloat("nearRadius:Blue", &nearRadius_, 0.1f);
	ImGui::DragFloat("halfAngle", &halfAngle_, 0.1f);
	ImGui::DragFloat("lerpTime", &lerpTime_, 0.01f);
	ImGui::DragFloat("fadeOutTime", &fadeOutTime_, 0.01f);
	ImGui::DragFloat("fadeInTime", &fadeInTime_, 0.01f);
	ImGui::DragFloat("emitParticleOffsetY", &emitParticleOffsetY_, 0.01f);
	Easing::SelectEasingType(easingType_);

	Vector3 center = player_->GetTranslation();
	center.y = 4.0f;
	LineRenderer::GetInstance()->DrawArc(8, farRadius_, halfAngle_,
		center, followCamera_->GetTransform().GetForward(), Color::Red());
	LineRenderer::GetInstance()->DrawArc(8, nearRadius_, halfAngle_,
		center, followCamera_->GetTransform().GetForward(), Color::Blue());
}

void BossEnemyTeleportationState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	farRadius_ = JsonAdapter::GetValue<float>(data, "farRadius_");
	nearRadius_ = JsonAdapter::GetValue<float>(data, "nearRadius_");
	halfAngle_ = JsonAdapter::GetValue<float>(data, "halfAngle_");
	lerpTime_ = JsonAdapter::GetValue<float>(data, "lerpTime_");
	fadeOutTime_ = JsonAdapter::GetValue<float>(data, "fadeOutTime_");
	fadeInTime_ = JsonAdapter::GetValue<float>(data, "fadeInTime_");
	emitParticleOffsetY_ = JsonAdapter::GetValue<float>(data, "emitParticleOffsetY_");
	easingType_ = static_cast<EasingType>(JsonAdapter::GetValue<int>(data, "easingType_"));

	{
		Json clampData;
		if (JsonAdapter::LoadCheck("GameConfig/gameConfig.json", clampData)) {

			moveClampSize_ = JsonAdapter::GetValue<float>(clampData["playableArea"], "length");
		}
	}
}

void BossEnemyTeleportationState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["farRadius_"] = farRadius_;
	data["nearRadius_"] = nearRadius_;
	data["halfAngle_"] = halfAngle_;
	data["lerpTime_"] = lerpTime_;
	data["fadeOutTime_"] = fadeOutTime_;
	data["fadeInTime_"] = fadeInTime_;
	data["emitParticleOffsetY_"] = emitParticleOffsetY_;
	data["easingType_"] = static_cast<int>(easingType_);
}