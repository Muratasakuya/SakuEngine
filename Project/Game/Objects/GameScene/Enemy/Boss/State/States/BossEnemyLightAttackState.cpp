#include "BossEnemyLightAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Effect/User/Helper/GameEffectCommandHelper.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyLightAttackState classMethods
//============================================================================

BossEnemyLightAttackState::BossEnemyLightAttackState(BossEnemy& bossEnemy) {

	// 剣エフェクト作成
	slash_.effect = std::make_unique<EffectGroup>();
	slash_.effect->Init("lightAttackSlash", "BossEnemyEffect");
	slash_.effect->LoadJson("GameEffectGroup/BossEnemy/bossEnemyLightAttackEffect_0.json");

	// 親の設定
	slash_.effect->SetParent("bossSlash_0", bossEnemy.GetTransform());
	slash_.effectNodeName = "bossSlash_0";
}

void BossEnemyLightAttackState::Enter(BossEnemy& bossEnemy) {

	// 攻撃予兆アニメーションを設定
	bossEnemy.SetNextAnimation("bossEnemy_lightAttackParrySign", false, nextAnimDuration_);

	// 座標を設定
	startPos_ = bossEnemy.GetTranslation();
	canExit_ = false;

	// 攻撃予兆を出す
	Vector3 sign = bossEnemy.GetTranslation();
	sign.y = 2.0f;
	attackSign_->Emit(Math::ProjectToScreen(sign, *followCamera_));

	// パリィ可能にする
	bossEnemy.ResetParryTiming();
	parryParam_.continuousCount = 1;
	parryParam_.canParry = true;

	parried_ = false;
}

void BossEnemyLightAttackState::Update(BossEnemy& bossEnemy) {

	// パリィ攻撃のタイミングを更新
	UpdateParryTiming(bossEnemy);

	// 状態に応じて更新
	switch (currentState_) {
	case BossEnemyLightAttackState::State::ParrySign: {

		// プレイヤーの方を向きながら補間
		UpdateParrySign(bossEnemy);
		break;
	}
	case BossEnemyLightAttackState::State::Attack: {

		// 攻撃、終了後状態を終了
		UpdateAttack(bossEnemy);
		break;
	}
	}
}

void BossEnemyLightAttackState::UpdateAlways(BossEnemy& bossEnemy) {

	// 剣エフェクトの更新、親の回転を設定する
	slash_.Update(bossEnemy);
}

void BossEnemyLightAttackState::UpdateParrySign(BossEnemy& bossEnemy) {

	// 目標座標を常に更新する
	const Vector3 playerPos = player_->GetTranslation();
	Vector3 direction = (bossEnemy.GetTranslation() - playerPos).Normalize();
	Vector3 target = playerPos - direction * attackOffsetTranslation_;
	target.y = 0.0f;
	LookTarget(bossEnemy, playerPos);

	// アニメーションが終了次第攻撃する
	if (bossEnemy.IsAnimationFinished()) {

		bossEnemy.SetNextAnimation("bossEnemy_lightAttack", false, nextAnimDuration_);

		// 状態を進める
		currentState_ = State::Attack;

		// 補間をリセット
		lerpTimer_ = 0.0f;
		startPos_ = bossEnemy.GetTranslation();
		reachedPlayer_ = false;
	}
}

void BossEnemyLightAttackState::UpdateAttack(BossEnemy& bossEnemy) {

	// 座標補間処理
	LerpTranslation(bossEnemy);

	// animationが終了したら経過時間を進める
	if (bossEnemy.IsAnimationFinished()) {

		exitTimer_ += GameTimer::GetDeltaTime();
		// 時間経過が過ぎたら遷移可能
		if (exitTime_ < exitTimer_) {

			canExit_ = true;
		}
	}
}

void BossEnemyLightAttackState::UpdateParryTiming(BossEnemy& bossEnemy) {

	// パリィ攻撃のタイミング
	switch (currentState_) {
	case BossEnemyLightAttackState::State::Attack: {
		if (bossEnemy.IsEventKey("Parry", 0)) {

			bossEnemy.TellParryTiming();
			parried_ = true;
		}
		break;
	}
	}
}

void BossEnemyLightAttackState::LerpTranslation(BossEnemy& bossEnemy) {

	// 目標座標計算
	const Vector3 playerPos = player_->GetTranslation();
	Vector3 direction = (bossEnemy.GetTranslation() - playerPos).Normalize();
	Vector3 target = playerPos - direction * attackOffsetTranslation_;
	target.y = 0.0f;

	if (!reachedPlayer_) {

		// プレイヤーの方を向くようにしておく
		LookTarget(bossEnemy, playerPos);

		// 補間時間を進める
		lerpTimer_ += GameTimer::GetScaledDeltaTime();
		float lerpT = std::clamp(lerpTimer_ / lerpTime_, 0.0f, 1.0f);
		lerpT = EasedValue(easingType_, lerpT);

		// 補間
		Vector3 newPos = Vector3::Lerp(startPos_, target, lerpT);
		bossEnemy.SetTranslation(newPos);

		// プレイヤーに十分近づいたら補間しない
		// xとzの距離を見る
		Vector2 distanceXZ = Vector2(playerPos.x - newPos.x, playerPos.z - newPos.z);
		if (distanceXZ.Length() <= std::fabs(attackOffsetTranslation_)) {

			reachedPlayer_ = true;
			bossEnemy.SetTranslation(target);

			// 剣エフェクトの発生
			slash_.Emit(bossEnemy);
		}
	}
}

void BossEnemyLightAttackState::Exit([[maybe_unused]] BossEnemy& bossEnemy) {

	// リセット
	canExit_ = false;
	reachedPlayer_ = false;
	parryParam_.canParry = false;
	lerpTimer_ = 0.0f;
	exitTimer_ = 0.0f;
	currentState_ = State::ParrySign;
	bossEnemy.ResetParryTiming();
}

void BossEnemyLightAttackState::ImGui(const BossEnemy& bossEnemy) {

	ImGui::Text(std::format("parried: {}", parried_).c_str());
	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("lerpTime", &lerpTime_, 0.001f);

	ImGui::DragFloat("attackOffsetTranslation", &attackOffsetTranslation_, 0.1f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);
	ImGui::DragFloat3("slashEffectOffset", &slash_.effectOffset.x, 0.1f);

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::Text("exitTimer: %.3f", exitTimer_);
	Easing::SelectEasingType(easingType_);

	// 座標を設定
	Vector3 start = bossEnemy.GetTranslation();
	const Vector3 playerPos = player_->GetTranslation();
	Vector3 direction = (start - playerPos).Normalize();
	Vector3 target = playerPos - direction * attackOffsetTranslation_;
	target.y = 2.0f;
	LineRenderer::GetInstance()->DrawSphere(8, 2.0f, target, Color::Red());
}

void BossEnemyLightAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	lerpTime_ = data.value("lerpTime_", 0.16f);

	attackOffsetTranslation_ = JsonAdapter::GetValue<float>(data, "attackOffsetTranslation_");
	exitTime_ = JsonAdapter::GetValue<float>(data, "exitTime_");
	easingType_ = static_cast<EasingType>(JsonAdapter::GetValue<int>(data, "easingType_"));

	slash_.effectOffset = Vector3::FromJson(data.value("slashEffectOffset_", Json()));
}

void BossEnemyLightAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["lerpTime_"] = lerpTime_;

	data["attackOffsetTranslation_"] = attackOffsetTranslation_;
	data["exitTime_"] = exitTime_;
	data["easingType_"] = static_cast<int>(easingType_);

	data["slashEffectOffset_"] = slash_.effectOffset.ToJson();
}