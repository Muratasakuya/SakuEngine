#include "BossEnemyProjectileAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	BossEnemyProjectileAttackState classMethods
//============================================================================

BossEnemyProjectileAttackState::BossEnemyProjectileAttackState(uint32_t phaseCount) {

	// エフェクトの初期化
	// 予備動作エフェクト
	preEffect_ = std::make_unique<EffectGroup>();
	preEffect_->Init("preProjectileEffect", "BossEnemyEffect");
	preEffect_->LoadJson("GameEffectGroup/BossEnemy/bossEnemyPreProjectileEffect.json");
	// 発生起動エフェクト
	launchEffect_ = std::make_unique<EffectGroup>();
	launchEffect_->Init("launchProjectileEffect", "BossEnemyEffect");
	launchEffect_->LoadJson("GameEffectGroup/BossEnemy/bossEnemylaunchProjectileEffect.json");
	// 弾エフェクト
	bulletEffect_ = std::make_unique<EffectGroup>();
	bulletEffect_->Init("bossEnemyProjectileBulletEffect", "BossEnemyEffect");
	bulletEffect_->LoadJson("GameEffectGroup/BossEnemy/bossEnemyProjectileBulletEffect.json");

	for (uint32_t index = 0; index < phaseCount + 1; ++index) {

		// 3..5..7..
		uint32_t count = 3 + index * 2;

		// フェーズに応じた弾の数
		phaseBulletCounts_.emplace_back(count);
	}
	canExit_ = false;
}

void BossEnemyProjectileAttackState::Enter(BossEnemy& bossEnemy) {

	// 初期状態を設定
	currentState_ = State::Pre;

	// 予備動作エフェクト開始
	preEffect_->Emit(bossEnemy.GetTranslation());
}

void BossEnemyProjectileAttackState::Update(BossEnemy& bossEnemy) {

	// 状態に応じて更新
	switch (currentState_) {
	case BossEnemyProjectileAttackState::State::Pre:

		// 予備動作更新
		UpdatePre(bossEnemy);
		break;
	case BossEnemyProjectileAttackState::State::Launch:

		// 発生起動更新
		UpdateLaunch(bossEnemy);
		break;
	case BossEnemyProjectileAttackState::State::Attack:

		// 攻撃更新
		UpdateAttack(bossEnemy);
		break;
	}
}

void  BossEnemyProjectileAttackState::UpdatePre(BossEnemy& bossEnemy) {

	// エフェクトの処理が終了したら次の状態に進ませる
	if (preEffect_->IsFinishedAllNode()) {

		// 次の状態へ
		currentState_ = State::Launch;

		// 発生起動エフェクト開始

	}
}

void  BossEnemyProjectileAttackState::UpdateLaunch(BossEnemy& bossEnemy) {

	// 発生時間を更新
	launchTimer_.Update();

	// 時間経過終了後、弾を発射させる
	if (launchTimer_.IsReached()) {

		// 次の状態へ
		currentState_ = State::Attack;
	}
}

void  BossEnemyProjectileAttackState::UpdateAttack(BossEnemy& bossEnemy) {

	// 攻撃時間を更新
	attackTimer_.Update();

	// 時間経過終了後、状態を終了可能にする
	if (attackTimer_.IsReached()) {

		canExit_ = true;
	}
}

void BossEnemyProjectileAttackState::SetLaunchPositions(const BossEnemy& bossEnemy, int32_t phaseIndex) {

	// 対象フェーズの弾数
	phaseIndex = std::clamp(phaseIndex, 0, static_cast<int32_t>(phaseBulletCounts_.size() - 1));
	uint32_t count = phaseBulletCounts_[phaseIndex];

	// 中心位置、Y座標は固定
	Vector3 center = bossEnemy.GetTranslation();
	center.y = launchTopPosY_;

	launchPositions_.clear();

	// 真ん中
	launchPositions_.emplace_back(center);

	// 半分の数
	uint32_t half = static_cast<uint32_t>(count / 2);

	// xは左右対称にするため絶対値
	Vector3 offset = Vector3(std::fabs(launchOffsetPos_.x), launchOffsetPos_.y, launchOffsetPos_.z);

	// 左右対象にオフセットをかける
	for (uint32_t i = 1; i <= half; ++i) {

		// オフセット計算
		Vector3 delta = offset * static_cast<float>(i);

		// 左右の発生位置
		Vector3 left = { center.x - delta.x, center.y + delta.y, center.z + delta.z };
		Vector3 right = { center.x + delta.x, center.y + delta.y, center.z + delta.z };

		// 追加
		launchPositions_.push_back(left);
		launchPositions_.push_back(right);
	}
}

void BossEnemyProjectileAttackState::UpdateAlways(BossEnemy& bossEnemy) {

	// エフェクトの更新
	preEffect_->Update();
	launchEffect_->Update();
	bulletEffect_->Update();
}

void BossEnemyProjectileAttackState::Exit(BossEnemy& bossEnemy) {

	// リセット
	canExit_ = false;
	launchTimer_.Reset();
	attackTimer_.Reset();
}

void BossEnemyProjectileAttackState::ImGui(const BossEnemy& bossEnemy) {

	ImGui::Text("currentState: %s", EnumAdapter<State>::ToString(currentState_));

	ImGui::DragInt("editingPhase", &editingPhase_, 1, 0, static_cast<uint32_t>(phaseBulletCounts_.size() - 1));

	ImGui::SeparatorText("Launch");

	ImGui::DragFloat("launchTopPosY", &launchTopPosY_, 0.01f);
	ImGui::DragFloat3("launchOffsetPos", &launchOffsetPos_.x, 0.01f);

	launchTimer_.ImGui("LaunchTimer");

	ImGui::SeparatorText("Attack");

	ImGui::DragFloat("bulletAttackDuration", &bulletAttackDuration_, 0.01f);

	ImGui::DragFloat3("editStartPos", &editStartPos_.x, 0.01f);
	ImGui::DragFloat3("editEndPos_", &editEndPos_.x, 0.01f);
	if (ImGui::Button("DebugEmitOnce")) {

		std::vector<Vector3> debugPositions;
		debugPositions.emplace_back(editStartPos_);
		debugPositions.emplace_back(editEndPos_);

		// デバッグ発生
		bulletEffect_->Emit(editStartPos_);
		bulletEffect_->SetKeyframePath(bulletParticleNodeKey_, debugPositions);
	}

	// 座標の更新
	SetLaunchPositions(bossEnemy, editingPhase_);

	// 現在発生させる位置のデバッグ表示
	for (const auto& pos : launchPositions_) {

		LineRenderer::GetInstance()->DrawSphere(6, 1.0f, pos, Color::Cyan());
	}
}

void BossEnemyProjectileAttackState::ApplyJson(const Json& data) {

	launchTimer_.FromJson(data.value("LaunchTimer", Json()));
	bulletAttackDuration_ = data.value("bulletAttackDuration", 1.0f);
	launchTopPosY_ = data.value("launchTopPosY", 4.0f);
	launchOffsetPos_ = Vector3::FromJson(data.value("launchOffsetPos", Json()));
}

void BossEnemyProjectileAttackState::SaveJson(Json& data) {

	launchTimer_.ToJson(data["LaunchTimer"]);
	data["bulletAttackDuration"] = bulletAttackDuration_;
	data["launchTopPosY"] = launchTopPosY_;
	data["launchOffsetPos"] = launchOffsetPos_.ToJson();
}