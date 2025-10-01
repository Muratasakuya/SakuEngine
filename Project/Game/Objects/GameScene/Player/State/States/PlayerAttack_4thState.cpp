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
		targetPos_ = startPos_ + player.GetTransform().GetForward() * moveValue_;
	}
}

void PlayerAttack_4thState::Update(Player& player) {

	// 範囲内にいるときは敵に向かって補間させる
	if (assisted_) {

		// 座標、回転補間
		AttackAssist(player);
	} else {

		// 前に前進させる
		moveTimer_.Update();
		Vector3 pos = Vector3::Lerp(startPos_, targetPos_, moveTimer_.easedT_);
		player.SetTranslation(pos);
	}

	// animationが終わったかチェック
	canExit_ = player.IsAnimationFinished();
	// animationが終わったら時間経過を進める
	if (canExit_) {

		exitTimer_ += GameTimer::GetScaledDeltaTime();
		// 画面シェイクを行わせる
		followCamera_->SetScreenShake(true);
	}
}

void PlayerAttack_4thState::Exit([[maybe_unused]] Player& player) {

	// リセット
	attackPosLerpTimer_ = 0.0f;
	exitTimer_ = 0.0f;
	moveTimer_.Reset();
}

void PlayerAttack_4thState::ImGui(const Player& player) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);

	PlayerBaseAttackState::ImGui(player);

	moveTimer_.ImGui("MoveTimer");
	ImGui::DragFloat("moveValue", &moveValue_, 0.1f);
}

void PlayerAttack_4thState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	exitTime_ = JsonAdapter::GetValue<float>(data, "exitTime_");

	PlayerBaseAttackState::ApplyJson(data);

	moveTimer_.FromJson(data.value("MoveTimer", Json()));
	moveValue_ = data.value("moveValue_", 1.0f);

	SetActionProgress();
}

void PlayerAttack_4thState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["exitTime_"] = exitTime_;

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
	int objectID = monitor->AddObject("PlayerAttack_4thState");

	// 全体進捗
	monitor->AddOverall(objectID, "Attack Progress", [this]() -> float {
		float progress = 0.0f;
		if (player_->GetCurrentAnimationName() == "player_attack_4th") {
			progress = player_->GetAnimationProgress();
		}
		return progress; });

	// 骨アニメーション（0→1 全域）
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
}