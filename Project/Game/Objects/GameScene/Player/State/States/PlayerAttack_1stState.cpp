#include "PlayerAttack_1stState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/ActionProgress/ActionProgressMonitor.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	PlayerAttack_1stState classMethods
//============================================================================

PlayerAttack_1stState::PlayerAttack_1stState(Player* player) {

	player_ = nullptr;
	player_ = player;
}

void PlayerAttack_1stState::Enter(Player& player) {

	player.SetNextAnimation("player_attack_1st", false, nextAnimDuration_);
	canExit_ = false;

	// 敵が攻撃可能範囲にいるかチェック
	const Vector3 playerPos = player.GetTranslation();
	// 補間座標を設定
	if (!CheckInRange(attackPosLerpCircleRange_,
		Vector3(bossEnemy_->GetTranslation() - playerPos).Length())) {

		startPos_ = playerPos;
		targetPos_ = startPos_ + player.GetTransform().GetForward() * moveValue_;
	}

	// 回転補間範囲内にいるかどうか
	assisted_ = CheckInRange(attackLookAtCircleRange_,
		Vector3(bossEnemy_->GetTranslation() - playerPos).Length());
	if (assisted_) {

		// カメラの向きを補正させる
		followCamera_->StartLookToTarget(FollowCameraTargetType::Player,
			FollowCameraTargetType::BossEnemy, true, true, targetCameraRotateX_);

		startPos_ = playerPos;
		targetPos_ = startPos_ + player.GetTransform().GetForward() * moveValue_;
	}
}

void PlayerAttack_1stState::Update(Player& player) {

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

		exitTimer_ += GameTimer::GetScaledDeltaTime();
	}
}

void PlayerAttack_1stState::Exit([[maybe_unused]] Player& player) {

	// リセット
	attackPosLerpTimer_ = 0.0f;
	exitTimer_ = 0.0f;
	moveTimer_.Reset();
}

void PlayerAttack_1stState::ImGui(const Player& player) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);
	ImGui::DragFloat("targetCameraRotateX", &targetCameraRotateX_, 0.01f);

	PlayerBaseAttackState::ImGui(player);

	moveTimer_.ImGui("MoveTimer");
	ImGui::DragFloat("moveValue", &moveValue_, 0.1f);
}

void PlayerAttack_1stState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	exitTime_ = JsonAdapter::GetValue<float>(data, "exitTime_");

	PlayerBaseAttackState::ApplyJson(data);

	moveTimer_.FromJson(data.value("MoveTimer", Json()));
	moveValue_ = data.value("moveValue_", 1.0f);
	targetCameraRotateX_ = data.value("targetCameraRotateX_", 0.0f);

	SetActionProgress();
}

void PlayerAttack_1stState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["exitTime_"] = exitTime_;

	PlayerBaseAttackState::SaveJson(data);

	moveTimer_.ToJson(data["MoveTimer"]);
	data["moveValue_"] = moveValue_;
	data["targetCameraRotateX_"] = targetCameraRotateX_;
}

bool PlayerAttack_1stState::GetCanExit() const {

	// 経過時間が過ぎたら
	bool canExit = exitTimer_ > exitTime_;
	return canExit;
}

void PlayerAttack_1stState::SetActionProgress() {

	ActionProgressMonitor* monitor = ActionProgressMonitor::GetInstance();
	int objectID = PlayerBaseAttackState::AddActionObject("PlayerAttack_1stState");

	// 全体進捗
	monitor->AddOverall(objectID, "AttackProgress_1st", [this]() -> float {
		float progress = 0.0f;
		if (player_->GetCurrentAnimationName() == "player_attack_1st") {
			progress = player_->GetAnimationProgress();
		}
		return progress; });

	// 攻撃骨アニメーション
	monitor->AddSpan(objectID, "Skinned Animation",
		[]() { return 0.0f; },
		[]() { return 1.0f; },
		[this]() {
			float progress = 0.0f;
			if (player_->GetCurrentAnimationName() == "player_attack_1st") {

				progress = player_->GetAnimationProgress();
			}
			return progress; });
	// 移動アニメーション
	monitor->AddSpan(objectID, "Move Animation",
		[]() { return 0.0f; },
		[this]() {
			float duration = player_->GetAnimationDuration("player_attack_1st");
			return this->moveTimer_.target_ / duration;
		},
		[this]() {
			return moveTimer_.t_;
		});

	// 進捗率の同期設定
	SetSpanUpdate(objectID);
}

void PlayerAttack_1stState::SetSpanUpdate(int objectID) {

	ActionProgressMonitor* monitor = ActionProgressMonitor::GetInstance();

	// 同期設定
	PlayerBaseAttackState::SetSynchObject(objectID);

	// 攻撃骨アニメーション
	monitor->SetSpanSetter(objectID, "Skinned Animation", [this](float t) {

		// アニメーションを切り替え
		if (player_->GetCurrentAnimationName() != "player_attack_1st") {

			player_->SetNextAnimation("player_attack_1st", false, 0.0f);
		}

		const float duration = player_->GetAnimationDuration("player_attack_1st");
		// アニメーションの時間を設定
		player_->SetCurrentAnimTime(duration * t);
		});

	// 移動アニメーション
	monitor->SetSpanSetter(objectID, "Move Animation", [this](float t) {

		// 補間値を設定
		moveTimer_.t_ = std::clamp(t, 0.0f, 1.0f);
		moveTimer_.easedT_ = EasedValue(moveTimer_.easeingType_, moveTimer_.t_);
		});
}