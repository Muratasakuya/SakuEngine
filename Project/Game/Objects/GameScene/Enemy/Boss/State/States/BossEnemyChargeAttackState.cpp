#include "BossEnemyChargeAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyChargeAttackState classMethods
//============================================================================

BossEnemyChargeAttackState::BossEnemyChargeAttackState() {

	// 1本の刃
	singleBlade_ = std::make_unique<BossEnemyBladeCollision>();
	singleBlade_->Init("singleBlade_Charge");
	// エフェクト
	// エフェクト、エンジン機能変更中...
	//singleBladeEffect_ = std::make_unique<BossEnemySingleBladeEffect>();
	//singleBladeEffect_->Init(singleBlade_->GetTransform(), "Charge");
}

void BossEnemyChargeAttackState::Enter(BossEnemy& bossEnemy) {

	bossEnemy.SetNextAnimation("bossEnemy_chargeAttack", false, nextAnimDuration_);

	canExit_ = false;
}

void BossEnemyChargeAttackState::UpdateAlways([[maybe_unused]] BossEnemy& bossEnemy) {

	// 衝突更新
	singleBlade_->Update();

	// エフェクトの更新処理
	// エフェクト、エンジン機能変更中...
	//singleBladeEffect_->Update();
}

void BossEnemyChargeAttackState::Update(BossEnemy& bossEnemy) {

	// 常にプレイヤーの方を向くようにする
	LookTarget(bossEnemy, player_->GetTranslation());
	// 刃の更新処理
	UpdateBlade(bossEnemy);

	if (bossEnemy.IsAnimationFinished()) {

		exitTimer_ += GameTimer::GetDeltaTime();
		// 時間経過が過ぎたら遷移可能
		if (exitTime_ < exitTimer_) {

			canExit_ = true;
		}
	}
}

void BossEnemyChargeAttackState::UpdateBlade(BossEnemy& bossEnemy) {

	// キーイベントで攻撃を発生させる
	if (bossEnemy.IsEventKey("Attack", 0)) {

		// 発生処理
		const Vector3 pos = bossEnemy.GetTranslation();
		const Vector3 velocity = CalcBaseDir(bossEnemy) * singleBladeMoveSpeed_;
		singleBlade_->EmitEffect(pos, velocity);

		// エフェクトを発生
		// エフェクト、エンジン機能変更中...
		/*singleBladeEffect_->EmitEffect(singleBlade_->GetTransform(),
			singleBladeEffectScalingValue_);*/
	}
}

Vector3 BossEnemyChargeAttackState::CalcBaseDir(const BossEnemy& bossEnemy) const {

	return (player_->GetTranslation() - bossEnemy.GetTranslation()).Normalize();
}

void BossEnemyChargeAttackState::Exit([[maybe_unused]] BossEnemy& bossEnemy) {

	// リセット
	exitTimer_ = 0.0f;
	canExit_ = false;
}

void BossEnemyChargeAttackState::ImGui([[maybe_unused]] const BossEnemy& bossEnemy) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);

	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);
	ImGui::DragFloat("singleBladeMoveSpeed", &singleBladeMoveSpeed_, 0.1f);

	if (ImGui::CollapsingHeader("Blade")) {
		if (ImGui::Button("Emit SingleBlade")) {

			// 発生処理
			const Vector3 pos = bossEnemy.GetTranslation();
			const Vector3 velocity = CalcBaseDir(bossEnemy) * singleBladeMoveSpeed_;
			singleBlade_->EmitEffect(pos, velocity);

			// エフェクトを発生
			// エフェクト、エンジン機能変更中...
			/*singleBladeEffect_->EmitEffect(singleBlade_->GetTransform(),
				singleBladeEffectScalingValue_);*/
		}

		ImGui::Separator();

		singleBlade_->ImGui();
		singleBlade_->Update();
	}

	if (ImGui::CollapsingHeader("Blade Effect")) {

		ImGui::DragFloat("singleBladeScaling", &singleBladeEffectScalingValue_, 0.01f);
		// エフェクト、エンジン機能変更中...
		/*singleBladeEffect_->ImGui();*/
	}
}

void BossEnemyChargeAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");

	exitTime_ = JsonAdapter::GetValue<float>(data, "exitTime_");
	singleBladeMoveSpeed_ = data.value("singleBladeMoveSpeed_", 1.0f);
	singleBladeEffectScalingValue_ = data.value("singleBladeEffectScalingValue_", 1.0f);
}

void BossEnemyChargeAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;

	data["exitTime_"] = exitTime_;
	data["singleBladeMoveSpeed_"] = singleBladeMoveSpeed_;
	data["singleBladeEffectScalingValue_"] = singleBladeEffectScalingValue_;
}