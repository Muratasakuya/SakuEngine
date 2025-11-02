#include "PlayerAttack_4thState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/ActionProgress/ActionProgressMonitor.h>
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	PlayerAttack_4thState classMethods
//============================================================================

PlayerAttack_4thState::PlayerAttack_4thState(Player* player) {

	player_ = nullptr;
	player_ = player;

	// エフェクト作成
	// 地割れエフェクト
	groundCrackEffect_ = std::make_unique<EffectGroup>();
	groundCrackEffect_->Init("groundCrack", "PlayerEffect");
	groundCrackEffect_->LoadJson("GameEffectGroup/Player/groundCrackEffect.json");
	groundCrackEmitted_ = false;

	// 回転エフェクト
	rotationEffect_ = std::make_unique<EffectGroup>();
	rotationEffect_->Init("rotationEffect", "PlayerEffect");
	rotationEffect_->LoadJson("GameEffectGroup/Player/playerAttack4thRotateEffect.json");

	// 親の設定
	rotationEffect_->SetParent("player4thAttackRotateSlash", player_->GetTransform());
	rotationEffect_->SetParent("player4thAttackRotateLightning", player_->GetTransform());
}

void PlayerAttack_4thState::Enter(Player& player) {

	player.SetNextAnimation("player_attack_4th", false, nextAnimDuration_);
	canExit_ = false;

	// 敵が攻撃可能範囲にいるかチェック
	const Vector3 playerPos = player.GetTranslation();
	assisted_ = CheckInRange(attackPosLerpCircleRange_,
		Vector3(bossEnemy_->GetTranslation() - playerPos).Length());

	// 補間座標を設定
	if (!assisted_) {

		startPos_ = playerPos;

		Vector3 forward = player.GetTransform().GetForward();
		forward.y = 0.0f;
		forward = forward.Normalize();
		targetPos_ = startPos_ + forward * moveValue_;
	}

	// 回転補間範囲内に入っていたら
	if (CheckInRange(attackLookAtCircleRange_,
		Vector3(bossEnemy_->GetTranslation() - playerPos).Length())) {

		// カメラアニメーション開始
		followCamera_->StartPlayerActionAnim(PlayerState::Attack_4th);
	}

	// 回転エフェクトの発生
	rotationEffect_->Emit(player_->GetRotation() * rotateEffectOffset_);
}

void PlayerAttack_4thState::Update(Player& player) {

	// 範囲内にいるときは敵に向かって補間させる
	if (assisted_) {

		// 座標、回転補間
		AttackAssist(player);
	} else {

		// 前に前進させる
		PlayerBaseAttackState::UpdateTimer(moveTimer_);
		Vector3 pos = Vector3::Lerp(startPos_, targetPos_, moveTimer_.easedT_);
		player.SetTranslation(pos);
	}

	// animationが終わったかチェック
	canExit_ = player.IsAnimationFinished();
	// animationが終わったら時間経過を進める
	if (canExit_) {

		// 発生していないときのみ
		if (!groundCrackEmitted_) {

			// 地割れエフェクトの発生
			// Y座標は固定
			Vector3 emitPos = player.GetTranslation();
			// 地面に隠れない位置に調整
			emitPos.y = 1.0f;
			groundCrackEffect_->Emit(emitPos);
			// 発生済みにする
			groundCrackEmitted_ = true;
		}

		// シェイク前にアニメーションを終了させる
		followCamera_->EndPlayerActionAnim(PlayerState::Attack_4th, false);

		exitTimer_ += GameTimer::GetScaledDeltaTime();
		// 画面シェイクを行わせる
		followCamera_->SetOverlayState(FollowCameraOverlayState::Shake, true);
	}
}

void PlayerAttack_4thState::UpdateAlways([[maybe_unused]] Player& player) {

	// 地割れエフェクトの更新
	groundCrackEffect_->Update();
	// 回転エフェクトの更新
	// 親の回転を設定する
	Quaternion offsetRotation =
		Quaternion::Normalize(Quaternion::EulerToQuaternion(rotateEffectOffsetRotation_));
	Quaternion rotation = Quaternion::Normalize(Quaternion::Multiply(player.GetRotation(), offsetRotation));

	rotationEffect_->SetParentRotation("player4thAttackRotateSlash", rotation, ParticleUpdateModuleID::Rotation);
	rotationEffect_->SetParentRotation("player4thAttackRotateLightning", rotation, ParticleUpdateModuleID::Primitive);
	rotationEffect_->Update();
}

void PlayerAttack_4thState::Exit([[maybe_unused]] Player& player) {

	// リセット
	attackPosLerpTimer_ = 0.0f;
	exitTimer_ = 0.0f;
	moveTimer_.Reset();
	groundCrackEmitted_ = false;

	// カメラアニメーションを終了させる
	followCamera_->EndPlayerActionAnim(PlayerState::Attack_4th, false);
}

void PlayerAttack_4thState::ImGui(const Player& player) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);

	ImGui::DragFloat3("rotateEffectOffset", &rotateEffectOffset_.x, 0.1f);
	ImGui::DragFloat3("rotateEffectOffsetRotation", &rotateEffectOffsetRotation_.x, 0.01f);

	PlayerBaseAttackState::ImGui(player);

	moveTimer_.ImGui("MoveTimer");
	ImGui::DragFloat("moveValue", &moveValue_, 0.1f);
}

void PlayerAttack_4thState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	exitTime_ = JsonAdapter::GetValue<float>(data, "exitTime_");

	rotateEffectOffset_ = Vector3::FromJson(data.value("rotateEffectOffset_", Json()));
	rotateEffectOffsetRotation_ = Vector3::FromJson(data.value("rotateEffectOffsetRotation_", Json()));

	PlayerBaseAttackState::ApplyJson(data);

	moveTimer_.FromJson(data.value("MoveTimer", Json()));
	moveValue_ = data.value("moveValue_", 1.0f);

	SetActionProgress();
}

void PlayerAttack_4thState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["exitTime_"] = exitTime_;

	data["rotateEffectOffset_"] = rotateEffectOffset_.ToJson();
	data["rotateEffectOffsetRotation_"] = rotateEffectOffsetRotation_.ToJson();

	PlayerBaseAttackState::SaveJson(data);

	moveTimer_.ToJson(data["MoveTimer"]);
	data["moveValue_"] = moveValue_;
}

bool PlayerAttack_4thState::GetCanExit() const {

	// 経過時間が過ぎたら
	bool canExit = exitTimer_ > exitTime_;
	return canExit;
}

void PlayerAttack_4thState::SetActionProgress() {

	ActionProgressMonitor* monitor = ActionProgressMonitor::GetInstance();
	int objectID = PlayerBaseAttackState::AddActionObject("PlayerAttack_4thState");

	// 全体進捗
	monitor->AddOverall(objectID, "AttackProgress_4th", [this]() -> float {
		float progress = 0.0f;
		if (player_->GetCurrentAnimationName() == "player_attack_4th") {
			progress = player_->GetAnimationProgress();
		}
		return progress; });

	// 骨アニメーション
	monitor->AddSpan(objectID, "Skinned Animation",
		[]() { return 0.0f; },
		[]() { return 1.0f; },
		[this]() {
			float progress = 0.0f;
			if (player_->GetCurrentAnimationName() == "player_attack_4th") {

				progress = player_->GetAnimationProgress();
			}
			return progress; });

	// 移動アニメーション
	monitor->AddSpan(objectID, "Move Animation",
		[]() { return 0.0f; },
		[this]() {
			float duration = player_->GetAnimationDuration("player_attack_4th");
			return moveTimer_.target_ / duration;
		},
		[this]() { return moveTimer_.t_; });

	// 進捗率の同期設定
	SetSpanUpdate(objectID);
}

void PlayerAttack_4thState::SetSpanUpdate(int objectID) {

	ActionProgressMonitor* monitor = ActionProgressMonitor::GetInstance();

	// 同期設定
	PlayerBaseAttackState::SetSynchObject(objectID);

	// 攻撃骨アニメーション
	monitor->SetSpanSetter(objectID, "Skinned Animation", [this](float t) {

		// アニメーションを切り替え
		if (player_->GetCurrentAnimationName() != "player_attack_4th") {

			player_->SetNextAnimation("player_attack_4th", false, 0.0f);
		}

		const float duration = player_->GetAnimationDuration("player_attack_4th");
		// アニメーションの時間を設定
		player_->SetCurrentAnimTime(duration * t);
		});

	// 移動アニメーション
	monitor->SetSpanSetter(objectID, "Move Animation", [this](float t) {

		// 補間値を設定
		moveTimer_.t_ = std::clamp(t, 0.0f, 1.0f);
		moveTimer_.easedT_ = EasedValue(moveTimer_.easeingType_, moveTimer_.t_); });
}