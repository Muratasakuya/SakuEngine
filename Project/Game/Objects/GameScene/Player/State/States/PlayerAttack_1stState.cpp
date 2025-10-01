#include "PlayerAttack_1stState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/ActionProgress/ActionProgressMonitor.h>
#include <Engine/Utility/Timer/GameTimer.h>
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
	assisted_ = CheckInRange(attackPosLerpCircleRange_,
		Vector3(bossEnemy_->GetTranslation() - playerPos).Length());

	// 補間座標を設定
	if (!assisted_) {

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
		moveTimer_.Update();
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

	ActionProgressMonitor* mon = ActionProgressMonitor::GetInstance();
	int obj = mon->AddObject("PlayerAttack_1stState"); // 一度だけ呼ぶガードは各自の設計で

	// Overall（全体）
	mon->AddOverall(obj, "Attack Progress", [this]() -> float {
		float p = player_->GetAnimationProgress();
		if (player_->IsAnimationFinished()) p = 1.0f;
		return std::clamp(p, 0.0f, 1.0f);
		});

	// Span: Anim Progress (raw) … 0→1 全域
	mon->AddSpan(obj, "Anim Progress (raw)",
		[]() { return 0.0f; }, []() { return 1.0f; },
		[this]() { return std::clamp(player_->GetAnimationProgress(), 0.0f, 1.0f); }
	);

	// Span: MoveTimer … 0→(target/duration)
	mon->AddSpan(obj, "MoveTimer",
		// start
		[]() { return 0.0f; },
		// end: target / duration
		[this]() {
			const std::string& name = player_->GetCurrentAnimationName();
			float duration = player_->GetAnimationDuration(name);
			if (duration <= 0.0f) return 0.0f;
			return std::clamp(this->moveTimer_.target_ / duration, 0.0f, 1.0f);
		},
		// local: 区間内進捗 = current / target（target>0）
		[this]() {
			float tgt = (std::max)(0.0f, this->moveTimer_.target_);
			if (tgt <= 0.0f) return 0.0f;
			float v = this->moveTimer_.current_ / tgt;
			return std::clamp(v, 0.0f, 1.0f);
		}
	);
}

void PlayerAttack_1stState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["exitTime_"] = exitTime_;

	PlayerBaseAttackState::SaveJson(data);

	moveTimer_.ToJson(data["MoveTimer"]);
	data["moveValue_"] = moveValue_;
}

bool PlayerAttack_1stState::GetCanExit() const {

	// 経過時間が過ぎたら
	bool canExit = exitTimer_ > exitTime_;
	return canExit;
}