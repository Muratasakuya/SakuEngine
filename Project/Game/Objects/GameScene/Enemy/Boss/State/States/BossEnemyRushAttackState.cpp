#include "BossEnemyRushAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyRushAttackState classMethods
//============================================================================

void BossEnemyRushAttackState::InitBlade() {

	// 3本の刃
	for (uint32_t index = 0; index < bladeMaxCount_; ++index) {

		divisionBlades_[index] = std::make_unique<BossEnemyBladeCollision>();
		divisionBlades_[index]->Init("divisionBlade");
	}

	// 1本の刃
	singleBlade_ = std::make_unique<BossEnemyBladeCollision>();
	singleBlade_->Init("singleBlade_Rush");
	// エフェクト
	// エフェクト、エンジン機能変更中...
	/*singleBladeEffect_ = std::make_unique<BossEnemySingleBladeEffect>();
	singleBladeEffect_->Init(singleBlade_->GetTransform(), "Rush");*/
}

BossEnemyRushAttackState::BossEnemyRushAttackState() {

	// 最大攻撃回数を設定
	maxAttackCount_ = static_cast<int>(pattern_.size());

	// 刃を初期化
	InitBlade();
}

void BossEnemyRushAttackState::Enter(BossEnemy& bossEnemy) {

	// 最初の設定
	currentState_ = State::Teleport;
	// テレポート状態でanimationを設定
	bossEnemy.SetNextAnimation("bossEnemy_teleport", false, nextAnimDuration_);
	canExit_ = false;

	// 座標設定
	Vector3 center = player_->GetTranslation();
	center.y = 0.0f;
	const Vector3 forward = followCamera_->GetTransform().GetForward();
	startPos_ = bossEnemy.GetTranslation();
	targetPos_ = Math::RandomPointOnArcInSquare(center, forward,
		farRadius_, halfAngle_, Vector3::AnyInit(0.0f), moveClampSize_ / 2.0f);

	currentAlpha_ = 1.0f;
	bossEnemy.SetAlpha(currentAlpha_);
	bossEnemy.SetCastShadow(true);

	// playerの方を向かせる
	LookTarget(bossEnemy, player_->GetTranslation());
}

void BossEnemyRushAttackState::UpdateAlways([[maybe_unused]] BossEnemy& bossEnemy) {

	// 衝突更新
	for (const auto& divisionBlade : divisionBlades_) {

		divisionBlade->Update();
	}
	singleBlade_->Update();

	// エフェクトの更新処理
	// エフェクト、エンジン機能変更中...
	//singleBladeEffect_->Update();
}

void BossEnemyRushAttackState::Update(BossEnemy& bossEnemy) {

	const float deltaTime = GameTimer::GetScaledDeltaTime();
	switch (currentState_) {
	case State::Teleport: {

		// テレポートの更新
		UpdateTeleport(bossEnemy, deltaTime);
		break;
	}
	case State::Attack: {

		// 攻撃更新
		UpdateAttack(bossEnemy);
		break;
	}
	case State::Cooldown: {

		// クールタイム更新
		UpdateCooldown(bossEnemy, deltaTime);
		break;
	}
	}

	// 衝突、刃の更新処理
	UpdateBlade(bossEnemy);

	// 攻撃回数が最大を超えたら遷移可能状態にする
	if (maxAttackCount_ <= currentAttackCount_) {

		canExit_ = true;
	}
}

void BossEnemyRushAttackState::UpdateTeleport(BossEnemy& bossEnemy, float deltaTime) {

	lerpTimer_ += deltaTime;
	float lerpT = std::clamp(lerpTimer_ / lerpTime_, 0.0f, 1.0f);
	lerpT = EasedValue(easingType_, lerpT);

	// 座標補完
	bossEnemy.SetTranslation(Vector3::Lerp(startPos_, targetPos_, lerpT));
	LookTarget(bossEnemy, player_->GetTranslation());

	const float disappearEnd = fadeOutTime_;           // 消え終わる時間
	const float appearStart = lerpTime_ - fadeInTime_; // 現れ始める時間

	bossEnemy.SetCastShadow(true);
	if (lerpTimer_ <= disappearEnd) {

		float t = std::clamp(lerpTimer_ / fadeOutTime_, 0.0f, 1.0f);
		currentAlpha_ = 1.0f - t;
	} else if (lerpTimer_ >= appearStart) {

		float t = std::clamp((lerpTimer_ - appearStart) / fadeInTime_, 0.0f, 1.0f);
		currentAlpha_ = t;
	} else {

		currentAlpha_ = 0.0f;
		bossEnemy.SetCastShadow(false);
	}
	bossEnemy.SetAlpha(currentAlpha_);

	// tが1.0fになったら攻撃animationに切り替える
	if (1.0f <= lerpT) {
		if (currentAttackCount_ < pattern_.size()) {

			// 攻撃アニメーションへ切り替え
			bossEnemy.SetNextAnimation(pattern_[currentAttackCount_].animationName, false, nextAnimDuration_);
			bossEnemy.SetTranslation(targetPos_);

			bossEnemy.SetAlpha(1.0f);
			bossEnemy.SetCastShadow(true);

			currentState_ = State::Attack;
			lerpTimer_ = 0.0f;
		}
	} else {

		Vector3 emitPos = bossEnemy.GetTranslation();
		emitPos.y = emitParticleOffsetY_;
	}
}

void BossEnemyRushAttackState::UpdateAttack(BossEnemy& bossEnemy) {

	// 攻撃animationが終了したら攻撃クールダウン状態に遷移させる
	if (bossEnemy.IsAnimationFinished()) {

		currentState_ = State::Cooldown;
		attackCoolTimer_ = 0.0f;
	}
}

void BossEnemyRushAttackState::UpdateCooldown(BossEnemy& bossEnemy, float deltaTime) {

	attackCoolTimer_ += deltaTime;
	if (attackCoolTime_ <= attackCoolTimer_) {

		// 次の攻撃に進める
		++currentAttackCount_;

		// テレポート設定
		currentState_ = State::Teleport;
		lerpTimer_ = 0.0f;

		// テレポート状態でanimationを設定
		bossEnemy.SetNextAnimation("bossEnemy_teleport", false, nextAnimDuration_);

		// 座標設定
		Vector3 center = player_->GetTranslation();
		center.y = 0.0f;
		const Vector3 forward = followCamera_->GetTransform().GetForward();
		startPos_ = bossEnemy.GetTranslation();
		targetPos_ = Math::RandomPointOnArcInSquare(center, forward,
			farRadius_, halfAngle_, Vector3::AnyInit(0.0f), moveClampSize_ / 2.0f);

		// playerの方を向かせる
		LookTarget(bossEnemy, player_->GetTranslation());
	}
}

void BossEnemyRushAttackState::UpdateBlade(BossEnemy& bossEnemy) {

	if (currentAttackCount_ == maxAttackCount_) {
		return;
	}

	// 最後の攻撃か判定
	bool isLastAttack = (currentAttackCount_ == maxAttackCount_ - 1);

	if (isLastAttack) {
		if (bossEnemy.IsEventKey("Attack", 0)) {

			EmitSingleBlade(bossEnemy);
		}
	} else {
		if (bossEnemy.IsEventKey("Attack", 0)) {

			EmitDivisionBlades(bossEnemy);
		}
	}
}

Vector3 BossEnemyRushAttackState::CalcBaseDir(const BossEnemy& bossEnemy) const {

	return (player_->GetTranslation() - bossEnemy.GetTranslation()).Normalize();
}

Vector3 BossEnemyRushAttackState::CalcDivisionBladeDir(const BossEnemy& bossEnemy, uint32_t index) const {

	const float offset[bladeMaxCount_] = { -divisionOffsetAngle_, 0.0f, divisionOffsetAngle_ };
	return Math::RotateY(CalcBaseDir(bossEnemy), offset[index] * pi / 180.0f);
}

void BossEnemyRushAttackState::EmitDivisionBlades(const BossEnemy& bossEnemy) {

	// 発生処理
	const Vector3 pos = bossEnemy.GetTranslation();
	for (uint32_t i = 0; i < bladeMaxCount_; ++i) {

		const Vector3 velocity = CalcDivisionBladeDir(bossEnemy, i) * divisionBladeMoveSpeed_;
		divisionBlades_[i]->EmitEffect(pos, velocity);
	}
}

void BossEnemyRushAttackState::EmitSingleBlade(const BossEnemy& bossEnemy) {

	// 発生処理
	const Vector3 pos = bossEnemy.GetTranslation();
	const Vector3 velocity = CalcBaseDir(bossEnemy) * singleBladeMoveSpeed_;
	singleBlade_->EmitEffect(pos, velocity);

	// エフェクトを発生
	// エフェクト、エンジン機能変更中...
	/*singleBladeEffect_->EmitEffect(singleBlade_->GetTransform(),
		singleBladeEffectScalingValue_);*/
}

void BossEnemyRushAttackState::Exit(BossEnemy& bossEnemy) {

	// リセット
	canExit_ = false;
	lerpTimer_ = 0.0f;
	attackCoolTimer_ = 0.0f;
	currentAttackCount_ = 0;
	currentAlpha_ = 1.0f;
	bossEnemy.SetAlpha(currentAlpha_);
	bossEnemy.SetCastShadow(true);
}

void BossEnemyRushAttackState::ImGui(const BossEnemy& bossEnemy) {

	if (ImGui::CollapsingHeader("RushAttackState")) {

		ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
		ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);

		ImGui::Text(std::format("canExit: {}", canExit_).c_str());
		ImGui::Text("currentAttack: %d / %d", currentAttackCount_, maxAttackCount_);
		ImGui::Text("attackCoolTime: %.3f / %.3f", attackCoolTimer_, attackCoolTime_);
		ImGui::DragFloat("farRadius:Red", &farRadius_, 0.1f);
		ImGui::DragFloat("nearRadius:Blue", &nearRadius_, 0.1f);
		ImGui::DragFloat("halfAngle", &halfAngle_, 0.1f);
		ImGui::DragFloat("lerpTime", &lerpTime_, 0.01f);
		ImGui::DragFloat("fadeOutTime", &fadeOutTime_, 0.01f);
		ImGui::DragFloat("fadeInTime", &fadeInTime_, 0.01f);
		ImGui::DragFloat("attackCoolTime", &attackCoolTime_, 0.01f);
		ImGui::DragFloat("emitParticleOffsetY_", &emitParticleOffsetY_, 0.01f);
		ImGui::DragFloat("divisionOffsetAngle_", &divisionOffsetAngle_, 0.01f);
		ImGui::DragFloat("divisionBladeMoveSpeed", &divisionBladeMoveSpeed_, 0.1f);
		ImGui::DragFloat("singleBladeMoveSpeed", &singleBladeMoveSpeed_, 0.1f);
		Easing::SelectEasingType(easingType_);

		{
			const Vector3 bossPos = bossEnemy.GetTranslation();
			const Vector3 toPlayer = (player_->GetTranslation() - bossPos).Normalize();
			const float   baseYaw = divisionOffsetAngle_ * (pi / 180.0f);
			const float   angles[bladeMaxCount_] = { -baseYaw, 0.0f, baseYaw };

			for (uint32_t i = 0; i < bladeMaxCount_; ++i) {
				Vector3 dir = Math::RotateY(toPlayer, angles[i]);
				Vector3 lineStart = bossPos + Vector3(0.0f, 4.0f, 0.0f);
				Vector3 lineEnd = lineStart + dir * 128.0f;

				LineRenderer::GetInstance()->DrawLine3D(lineStart, lineEnd, Color::Red());
			}
		}

		{
			Vector3 center = player_->GetTranslation();
			center.y = 4.0f;
			LineRenderer::GetInstance()->DrawArc(8, farRadius_, halfAngle_,
				center, followCamera_->GetTransform().GetForward(), Color::Red());
			LineRenderer::GetInstance()->DrawArc(8, nearRadius_, halfAngle_,
				center, followCamera_->GetTransform().GetForward(), Color::Blue());
		}
	}

	if (ImGui::CollapsingHeader("Blade")) {

		// 発生させる
		if (ImGui::Button("Emit DivisionBlade")) {

			EmitDivisionBlades(bossEnemy);
		}
		if (ImGui::Button("Emit SingleBlade")) {

			EmitSingleBlade(bossEnemy);
		}

		ImGui::Separator();

		// 真ん中
		divisionBlades_[1]->ImGui();
		singleBlade_->ImGui();

		// 衝突更新
		for (const auto& divisionBlade : divisionBlades_) {

			divisionBlade->Update();
		}
		singleBlade_->Update();
	}

	if (ImGui::CollapsingHeader("Blade Effect")) {

		ImGui::DragFloat("singleBladeScaling", &singleBladeEffectScalingValue_, 0.01f);
		// エフェクト、エンジン機能変更中...
		//singleBladeEffect_->ImGui();
	}
}

void BossEnemyRushAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	farRadius_ = JsonAdapter::GetValue<float>(data, "farRadius_");
	nearRadius_ = JsonAdapter::GetValue<float>(data, "nearRadius_");
	halfAngle_ = JsonAdapter::GetValue<float>(data, "halfAngle_");
	lerpTime_ = JsonAdapter::GetValue<float>(data, "lerpTime_");
	attackCoolTime_ = JsonAdapter::GetValue<float>(data, "attackCoolTime_");
	fadeOutTime_ = JsonAdapter::GetValue<float>(data, "fadeOutTime_");
	fadeInTime_ = JsonAdapter::GetValue<float>(data, "fadeInTime_");
	emitParticleOffsetY_ = JsonAdapter::GetValue<float>(data, "emitParticleOffsetY_");
	divisionOffsetAngle_ = JsonAdapter::GetValue<float>(data, "divisionOffsetAngle_");
	divisionBladeMoveSpeed_ = JsonAdapter::GetValue<float>(data, "divisionBladeMoveSpeed_");
	singleBladeMoveSpeed_ = JsonAdapter::GetValue<float>(data, "singleBladeMoveSpeed_");
	easingType_ = static_cast<EasingType>(JsonAdapter::GetValue<int>(data, "easingType_"));
	singleBladeEffectScalingValue_ = data.value("singleBladeEffectScalingValue_", 1.0f);

	{
		Json clampData;
		if (JsonAdapter::LoadCheck("GameConfig/gameConfig.json", clampData)) {

			moveClampSize_ = JsonAdapter::GetValue<float>(clampData["playableArea"], "length");
		}
	}
}

void BossEnemyRushAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["farRadius_"] = farRadius_;
	data["nearRadius_"] = nearRadius_;
	data["halfAngle_"] = halfAngle_;
	data["lerpTime_"] = lerpTime_;
	data["attackCoolTime_"] = attackCoolTime_;
	data["fadeOutTime_"] = fadeOutTime_;
	data["fadeInTime_"] = fadeInTime_;
	data["emitParticleOffsetY_"] = emitParticleOffsetY_;
	data["divisionOffsetAngle_"] = divisionOffsetAngle_;
	data["divisionBladeMoveSpeed_"] = divisionBladeMoveSpeed_;
	data["singleBladeMoveSpeed_"] = singleBladeMoveSpeed_;
	data["singleBladeEffectScalingValue_"] = singleBladeEffectScalingValue_;
	data["easingType_"] = static_cast<int>(easingType_);
}