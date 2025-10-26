#include "BossEnemyJumpAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Animation/LerpKeyframe.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	BossEnemyJumpAttackState classMethods
//============================================================================

BossEnemyJumpAttackState::BossEnemyJumpAttackState() {

	// サイズ分確保
	float offsetY = 2.0f;
	for (uint32_t i = 0; i < jumpKeyframeCount_; ++i) {

		// iでオフセット
		float posY = offsetY * static_cast<float>(i);
		jumpKeyframes_.emplace_back(Vector3(0.0f, posY, 0.0f));
	}
}

void BossEnemyJumpAttackState::Enter(BossEnemy& bossEnemy) {

	// 予備動作アニメーションの再生
	bossEnemy.SetNextAnimation("bossEnemy_jumpPrepare", false, nextAnimDuration_);

	// 最初の状態で初期化
	currentState_ = State::Pre;
}

void BossEnemyJumpAttackState::Update(BossEnemy& bossEnemy) {

	// 状態に応じて更新
	switch (currentState_) {
	case BossEnemyJumpAttackState::State::Pre:

		// 予備動作
		UpdatePre(bossEnemy);
		break;
	case BossEnemyJumpAttackState::State::Jump:

		// ジャンプ
		UpdateJump(bossEnemy);
		break;
	}
}

void BossEnemyJumpAttackState::UpdatePre(BossEnemy& bossEnemy) {

	// 予備動作中はプレイヤーの方を向く
	LookTarget(bossEnemy, player_->GetTranslation());

	// 予備動作が終了したらジャンプ状態へ
	if (bossEnemy.IsAnimationFinished()) {

		// ジャンプアニメーションの再生
		bossEnemy.SetNextAnimation("bossEnemy_jumpAttack", false, nextAnimDuration_);

		// ジャンプ状態へ
		currentState_ = State::Jump;

		// 補間座標の設定
		// 開始座標
		lerpTranslationXZ_.SetStart(bossEnemy.GetTranslation());
		// 終了座標、プレイヤーの位置
		lerpTranslationXZ_.SetEnd(player_->GetTranslation());
	}
}

void BossEnemyJumpAttackState::UpdateJump(BossEnemy& bossEnemy) {

	// 座標を補間
	Vector3 translation = bossEnemy.GetTranslation();
	lerpTranslationXZ_.LerpValue(translation);

	// Y座標をジャンプ補間、Y座標のみ返す補間
	translation.y = LerpKeyframe::GetValue(jumpKeyframes_,
		EasedValue(jumpEasing_, lerpTranslationXZ_.GetProgress()), LerpKeyframe::Type::Spline).y;

	// 座標を設定
	bossEnemy.SetTranslation(translation);

	//補間が終了したら状態を終了
	if (lerpTranslationXZ_.IsFinished()) {

		canExit_ = true;
	}
}

void BossEnemyJumpAttackState::UpdateAlways([[maybe_unused]] BossEnemy& bossEnemy) {
}

void BossEnemyJumpAttackState::Exit([[maybe_unused]] BossEnemy& bossEnemy) {

	// リセット
	canExit_ = false;
	lerpTranslationXZ_.Reset();
}

void BossEnemyJumpAttackState::ImGui([[maybe_unused]] const BossEnemy& bossEnemy) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.01f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.01f);

	lerpTranslationXZ_.ImGui("LerpTranslationXZ");

	ImGui::SeparatorText("Jump");

	for (uint32_t index = 0; index < jumpKeyframeCount_; ++index) {

		ImGui::PushID(index);

		Vector3& keyframe = jumpKeyframes_[index];
		const std::string key = "Keyframe: " + std::to_string(index);

		ImGui::DragFloat3(key.c_str(), &keyframe.x, 0.01f);

		ImGui::PopID();
	}

	EnumAdapter<EasingType>::Combo("JumpEasing", &jumpEasing_);
}

void BossEnemyJumpAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = data.value("nextAnimDuration_", 0.08f);
	rotationLerpRate_ = data.value("rotationLerpRate_", 0.08f);
	lerpTranslationXZ_.FromJson(data.value("LerpTranslationXZ", Json()));
	jumpEasing_ = EnumAdapter<EasingType>::FromString(data.value("jumpEasing_", "Linear")).value();
}

void BossEnemyJumpAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	lerpTranslationXZ_.ToJson(data["LerpTranslationXZ"]);
	data["jumpEasing_"] = EnumAdapter<EasingType>::ToString(jumpEasing_);
}