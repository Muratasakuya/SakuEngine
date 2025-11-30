#include "BossEnemyStrongAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyStrongAttackState classMethods
//============================================================================

BossEnemyStrongAttackState::BossEnemyStrongAttackState(BossEnemy& bossEnemy) {

	parriedMaps_[State::Attack1st] = false;
	parriedMaps_[State::Attack2nd] = false;

	// 剣エフェクト作成
	// 強攻撃エフェクト
	strongSlash_.effect = std::make_unique<EffectGroup>();
	strongSlash_.effect->Init("strongAttackSlash", "BossEnemyEffect");
	strongSlash_.effect->LoadJson("GameEffectGroup/BossEnemy/bossEnemyStrongAttackEffect.json");
	// 弱攻撃エフェクト
	lightSlash_.effect = std::make_unique<EffectGroup>();
	lightSlash_.effect->Init("lightAttackSlash", "BossEnemyEffect");
	lightSlash_.effect->LoadJson("GameEffectGroup/BossEnemy/bossEnemyLightAttackEffect_1.json");

	// 親の設定
	strongSlash_.effect->SetParent("bossSlash_1", bossEnemy.GetTransform());
	strongSlash_.effectNodeName = "bossSlash_1";
	lightSlash_.effect->SetParent("bossSlash_0", bossEnemy.GetTransform());
	lightSlash_.effectNodeName = "bossSlash_0";
}

void BossEnemyStrongAttackState::Enter(BossEnemy& bossEnemy) {

	// 攻撃予兆アニメーションを設定
	bossEnemy.SetNextAnimation("bossEnemy_strongAttackParrySign", false, nextAnimDuration_);

	// 座標を設定
	startPos_ = bossEnemy.GetTranslation();
	canExit_ = false;

	// 攻撃予兆を出す
	Vector3 sign = bossEnemy.GetTranslation();
	sign.y = 2.0f;
	attackSign_->Emit(Math::ProjectToScreen(sign, *followCamera_));

	// パリィ可能にする
	bossEnemy.ResetParryTiming();
	parryParam_.continuousCount = 2;
	parryParam_.canParry = true;

	parriedMaps_[State::Attack1st] = false;
	parriedMaps_[State::Attack2nd] = false;
}

void BossEnemyStrongAttackState::Update(BossEnemy& bossEnemy) {

	// パリィ攻撃のタイミングを更新
	UpdateParryTiming(bossEnemy);

	// 状態に応じて更新
	switch (currentState_) {
	case BossEnemyStrongAttackState::State::ParrySign: {

		// プレイヤーの方を向きながら補間
		UpdateParrySign(bossEnemy);
		break;
	}
	case BossEnemyStrongAttackState::State::Attack1st: {

		// 攻撃1回目
		UpdateAttack1st(bossEnemy);
		break;
	}
	case BossEnemyStrongAttackState::State::Attack2nd: {

		// 攻撃2回目
		UpdateAttack2nd(bossEnemy);
		break;
	}
	}
}

void BossEnemyStrongAttackState::UpdateAlways(BossEnemy& bossEnemy) {

	// エフェクト更新
	strongSlash_.Update(bossEnemy);
	lightSlash_.Update(bossEnemy);
}

void BossEnemyStrongAttackState::UpdateParrySign(BossEnemy& bossEnemy) {

	// 目標座標を常に更新する
	const Vector3 playerPos = player_->GetTranslation();
	Vector3 direction = (bossEnemy.GetTranslation() - playerPos).Normalize();
	Vector3 target = playerPos - direction * attackOffsetTranslation_;
	target.y = 0.0f;
	LookTarget(bossEnemy, playerPos);

	// アニメーションが終了次第攻撃する
	if (bossEnemy.IsAnimationFinished()) {

		bossEnemy.SetNextAnimation("bossEnemy_strongAttack", false, nextAnimDuration_);

		// 状態を進める
		currentState_ = State::Attack1st;

		// 座標を設定
		startPos_ = bossEnemy.GetTranslation();

		// 剣エフェクト発生
		strongSlash_.Emit(bossEnemy);
	}
}

void BossEnemyStrongAttackState::UpdateAttack1st(BossEnemy& bossEnemy) {

	// 座標補間処理
	LerpTranslation(bossEnemy);

	// animationが終了したら次の攻撃に移る
	if (bossEnemy.IsAnimationFinished()) {

		bossEnemy.SetNextAnimation("bossEnemy_lightAttack", false, attack2ndAnimDuration_);

		// 状態を進める
		currentState_ = State::Attack2nd;

		// 補間をリセット
		lerpTimer_ = 0.0f;
		startPos_ = bossEnemy.GetTranslation();
		reachedPlayer_ = false;

		// 剣エフェクト発生
		lightSlash_.Emit(bossEnemy);
	}
}

void BossEnemyStrongAttackState::UpdateAttack2nd(BossEnemy& bossEnemy) {

	// 座標補間処理
	LerpTranslation(bossEnemy);

	// animationが終了したら経過時間を進める
	if (bossEnemy.IsAnimationFinished()) {

		exitTimer_ += GameTimer::GetScaledDeltaTime();
		// 時間経過が過ぎたら遷移可能
		if (exitTime_ < exitTimer_) {

			canExit_ = true;
		}
	}
}

void BossEnemyStrongAttackState::UpdateParryTiming(BossEnemy& bossEnemy) {

	// パリィ攻撃のタイミング
	switch (currentState_) {
	case BossEnemyStrongAttackState::State::Attack1st: {

		if (bossEnemy.IsEventKey("Parry", 0)) {

			bossEnemy.TellParryTiming();
			parriedMaps_[currentState_] = true;
		}
		break;
	}
	case BossEnemyStrongAttackState::State::Attack2nd: {

		if (bossEnemy.IsEventKey("Parry", 0)) {

			bossEnemy.TellParryTiming();
			parriedMaps_[currentState_] = true;
		}
		break;
	}
	}
}

void BossEnemyStrongAttackState::LerpTranslation(BossEnemy& bossEnemy) {

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
		float lerpT = std::clamp(lerpTimer_ / attack2ndLerpTime_, 0.0f, 1.0f);
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
		}
	}
}

void BossEnemyStrongAttackState::Exit([[maybe_unused]] BossEnemy& bossEnemy) {

	// リセット
	canExit_ = false;
	reachedPlayer_ = false;
	parryParam_.canParry = false;
	lerpTimer_ = 0.0f;
	exitTimer_ = 0.0f;
	currentState_ = State::ParrySign;
	bossEnemy.ResetParryTiming();
}

void BossEnemyStrongAttackState::ImGui([[maybe_unused]] const BossEnemy& bossEnemy) {

	ImGui::Text(std::format("parried: Attack1st: {}", parriedMaps_[State::Attack1st]).c_str());
	ImGui::Text(std::format("parried: Attack2nd: {}", parriedMaps_[State::Attack2nd]).c_str());
	ImGui::Text(std::format("reachedPlayer: {}", reachedPlayer_).c_str());

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("attack2ndAnimDuration", &attack2ndAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);

	ImGui::DragFloat("attackOffsetTranslation", &attackOffsetTranslation_, 0.1f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);
	ImGui::DragFloat("attack2ndLerpTime", &attack2ndLerpTime_, 0.01f);

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::Text("exitTimer: %.3f", exitTimer_);
	Easing::SelectEasingType(easingType_);

	ImGui::DragFloat3("strongSlashOffset", &strongSlash_.effectOffset.x, 0.1f);
	ImGui::DragFloat3("lightSlashOffset", &lightSlash_.effectOffset.x, 0.1f);

	// 座標を設定
	// 座標を設定
	Vector3 start = bossEnemy.GetTranslation();
	const Vector3 playerPos = player_->GetTranslation();
	Vector3 direction = (start - playerPos).Normalize();
	Vector3 target = playerPos - direction * attackOffsetTranslation_;
	target.y = 2.0f;
	LineRenderer::GetInstance()->DrawSphere(8, 2.0f, target, Color::Red());

	ImGui::Text(std::format("(playerPos - start).Length(): {}", (playerPos - start).Length()).c_str());
}

void BossEnemyStrongAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	attack2ndAnimDuration_ = data.value("attack2ndAnimDuration_", 0.4f);
	attack2ndLerpTime_ = data.value("attack2ndLerpTime_", 0.4f);
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");

	attackOffsetTranslation_ = JsonAdapter::GetValue<float>(data, "attackOffsetTranslation_");
	exitTime_ = JsonAdapter::GetValue<float>(data, "exitTime_");
	easingType_ = static_cast<EasingType>(JsonAdapter::GetValue<int>(data, "easingType_"));

	strongSlash_.effectOffset = Vector3::FromJson(data.value("strongSlashOffset", Json()));
	lightSlash_.effectOffset = Vector3::FromJson(data.value("lightSlashOffset", Json()));
}

void BossEnemyStrongAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["attack2ndAnimDuration_"] = attack2ndAnimDuration_;
	data["attack2ndLerpTime_"] = attack2ndLerpTime_;
	data["rotationLerpRate_"] = rotationLerpRate_;

	data["attackOffsetTranslation_"] = attackOffsetTranslation_;
	data["exitTime_"] = exitTime_;
	data["easingType_"] = static_cast<int>(easingType_);

	data["strongSlashOffset"] = strongSlash_.effectOffset.ToJson();
	data["lightSlashOffset"] = lightSlash_.effectOffset.ToJson();
}