#include "PlayerAttack_2ndState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/ActionProgress/ActionProgressMonitor.h>
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	PlayerAttack_2ndState classMethods
//============================================================================

PlayerAttack_2ndState::PlayerAttack_2ndState(Player* player) {

	player_ = nullptr;
	player_ = player;
}

void PlayerAttack_2ndState::Enter(Player& player) {

	player.SetNextAnimation("player_attack_2nd", false, nextAnimDuration_);
	canExit_ = false;

	// 距離を比較
	const float dist = (bossEnemy_->GetTranslation() - player.GetTranslation()).Length();
	approachPhase_ = (dist > attackPosLerpCircleRange_);

	// 初期化
	currentIndex_ = 0;
	segmentTimer_ = 0.0f;
	segmentTime_ = attackPosLerpTime_ / 3.0f;

	if (approachPhase_) {

		// 範囲外のとき
		CalcApproachWayPoints(player, wayPoints_);
		startTranslation_ = player.GetTranslation();
	} else {

		// 範囲内のとき
		CalcWayPoints(player, wayPoints_);
		startTranslation_ = player.GetTranslation();
	}
}

void PlayerAttack_2ndState::Update(Player& player) {

	if (player.GetUpdateMode() == ObjectUpdateMode::External) {
		return;
	}

	// animationが終わったかチェック
	canExit_ = player.IsAnimationFinished();
	// animationが終わったら時間経過を進める
	if (canExit_) {

		exitTimer_ += GameTimer::GetScaledDeltaTime();
	}

	// 区間補間処理
	const bool finished = LerpAlongSegments(player);
	if (!finished) {
		return;
	}

	// 区間完了ごとに次の経路を決める
	if (approachPhase_) {

		if (!loopApproach_) {
			if (targetTranslation_.has_value()) {

				player.SetTranslation(*targetTranslation_);
			}
			approachPhase_ = false; // これ以上補間処理を行わない
			return;
		}

		// 範囲内になったら敵補間へ切り替え
		const float dist = (bossEnemy_->GetTranslation() - player.GetTranslation()).Length();
		if (dist <= attackPosLerpCircleRange_) {

			approachPhase_ = false;
			CalcWayPoints(player, wayPoints_);
			startTranslation_ = player.GetTranslation();
		} else {

			// 範囲外
			CalcApproachWayPoints(player, wayPoints_);
			startTranslation_ = player.GetTranslation();
		}

		// リセット
		currentIndex_ = 0;
		segmentTimer_ = 0.0f;
		segmentTime_ = attackPosLerpTime_ / 3.0f;
	} else {

		// 範囲内の経路完走後は最終点へ固定
		player.SetTranslation(*targetTranslation_);
	}
}

void PlayerAttack_2ndState::CalcWayPoints(const Player& player, std::array<Vector3, kNumSegments>& dstWayPoints) {

	// 目標座標を設定
	startTranslation_ = player.GetTranslation();
	const Vector3 enemyPos = bossEnemy_->GetTranslation();
	Vector3 dir = (enemyPos - startTranslation_).Normalize();
	targetTranslation_ = enemyPos - dir * attackOffsetTranslation_;

	//距離に応じて振れ幅を変更する
	const float distance = (*targetTranslation_ - startTranslation_).Length();
	const float swayLength = (std::max)(0.0f, (attackPosLerpCircleRange_ - distance)) * swayRate_;

	// 補間先を設定する
	CalcWayPointsToTarget(startTranslation_, *targetTranslation_,
		leftPointAngle_, rightPointAngle_, swayLength, dstWayPoints);
}

void PlayerAttack_2ndState::CalcWayPointsToTarget(const Vector3& start, const Vector3& target,
	float leftT, float rightT, float swayLength, std::array<Vector3, kNumSegments>& dstWayPoints) {

	Vector3 dir = (target - start).Normalize();
	Vector3 right = Vector3::Cross(dir, Vector3(0.0f, 1.0f, 0.0f)).Normalize();

	dstWayPoints[0] = Vector3::Lerp(start, target, leftT) - right * swayLength;  // 左
	dstWayPoints[1] = Vector3::Lerp(start, target, rightT) + right * swayLength; // 右
	dstWayPoints[2] = target;
}

void PlayerAttack_2ndState::CalcApproachWayPoints(const Player& player,
	std::array<Vector3, kNumSegments>& dstWayPoints) {

	// プレイヤーの前方向に向かってジグザグ移動させる
	startTranslation_ = player.GetTranslation();
	Vector3 forward = player.GetTransform().GetForward().Normalize();
	Vector3 target = startTranslation_ + forward * approachForwardDistance_;
	targetTranslation_ = target;

	// 補間先を設定する
	CalcWayPointsToTarget(startTranslation_, target,
		approachLeftPointAngle_, approachRightPointAngle_,
		approachSwayLength_, dstWayPoints);
}

bool PlayerAttack_2ndState::LerpAlongSegments(Player& player) {

	if (wayPoints_.size() <= currentIndex_) {
		if (targetTranslation_.has_value()) {

			player.SetTranslation(*targetTranslation_);
		}
		return true;
	}

	// 区間更新
	segmentTimer_ += GameTimer::GetScaledDeltaTime();
	float t = std::clamp(segmentTimer_ / segmentTime_, 0.0f, 1.0f);
	t = EasedValue(attackPosEaseType_, t);

	Vector3 segStart = (currentIndex_ == 0) ? startTranslation_ : wayPoints_[currentIndex_ - 1];
	Vector3 segEnd = wayPoints_[currentIndex_];

	Vector3 pos = Vector3::Lerp(segStart, segEnd, t);
	player.SetTranslation(pos);

	// 補間が終了したら次の区間に進める
	if (segmentTime_ <= segmentTimer_) {

		segmentTimer_ = 0.0f;
		++currentIndex_;
		return (currentIndex_ >= wayPoints_.size());
	}
	return false;
}

void PlayerAttack_2ndState::Exit([[maybe_unused]] Player& player) {

	// リセット
	attackPosLerpTimer_ = 0.0f;
	exitTimer_ = 0.0f;
}

void PlayerAttack_2ndState::ImGui(const Player& player) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);
	ImGui::DragFloat("swayRate", &swayRate_, 0.01f);
	ImGui::DragFloat("leftPointAngle", &leftPointAngle_, 0.01f);
	ImGui::DragFloat("rightPointAngle", &rightPointAngle_, 0.01f);

	PlayerBaseAttackState::ImGui(player);

	// 範囲内
	CalcWayPoints(player, debugWayPoints_);
	{
		LineRenderer* renderer = LineRenderer::GetInstance();
		Vector3 prev = player.GetTranslation();
		for (auto& p : debugWayPoints_) {
			p.y = 2.0f;
			renderer->DrawSphere(8, 2.0f, p, Color::Red());
			renderer->DrawLine3D(p, p + Vector3(0, 2, 0), Color::White());
			renderer->DrawLine3D(prev, p, Color::White());
			prev = p;
		}
	}

	// 範囲外
	ImGui::SeparatorText("Approach (Out of Range)");
	ImGui::DragFloat("approachForwardDistance", &approachForwardDistance_, 0.1f);
	ImGui::DragFloat("approachSwayLength", &approachSwayLength_, 0.01f);
	ImGui::DragFloat("approachLeftPointAngle", &approachLeftPointAngle_, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("approachRightPointAngle", &approachRightPointAngle_, 0.01f, 0.0f, 1.0f);

	CalcApproachWayPoints(player, debugApproachWayPoints_);
	{
		LineRenderer* renderer = LineRenderer::GetInstance();
		Vector3 prev = player.GetTranslation();
		for (auto& p : debugApproachWayPoints_) {

			p.y = 2.0f;
			renderer->DrawSphere(8, 2.0f, p, Color::Green());
			renderer->DrawLine3D(p, p + Vector3(0, 2, 0), Color::White());
			renderer->DrawLine3D(prev, p, Color::White());
			prev = p;
		}
	}
}

void PlayerAttack_2ndState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	exitTime_ = JsonAdapter::GetValue<float>(data, "exitTime_");
	swayRate_ = JsonAdapter::GetValue<float>(data, "swayRate_");
	leftPointAngle_ = JsonAdapter::GetValue<float>(data, "leftPointAngle_");
	rightPointAngle_ = JsonAdapter::GetValue<float>(data, "rightPointAngle_");

	approachForwardDistance_ = data.value("approachForwardDistance_", approachForwardDistance_);
	approachSwayLength_ = data.value("approachSwayLength_", approachSwayLength_);
	approachLeftPointAngle_ = data.value("approachLeftPointAngle_", approachLeftPointAngle_);
	approachRightPointAngle_ = data.value("approachRightPointAngle_", approachRightPointAngle_);

	PlayerBaseAttackState::ApplyJson(data);

	SetActionProgress();
}

void PlayerAttack_2ndState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["exitTime_"] = exitTime_;
	data["swayRate_"] = swayRate_;
	data["leftPointAngle_"] = leftPointAngle_;
	data["rightPointAngle_"] = rightPointAngle_;

	data["approachForwardDistance_"] = approachForwardDistance_;
	data["approachSwayLength_"] = approachSwayLength_;
	data["approachLeftPointAngle_"] = approachLeftPointAngle_;
	data["approachRightPointAngle_"] = approachRightPointAngle_;

	PlayerBaseAttackState::SaveJson(data);
}

bool PlayerAttack_2ndState::GetCanExit() const {

	// 経過時間が過ぎたら
	bool canExit = exitTimer_ > exitTime_;
	return canExit;
}

void PlayerAttack_2ndState::DriveOverall(float overall) {

	// 外部同期中は終了ガード系を解除（巻き戻し直後に止まらないように）
	canExit_ = false;
	exitTimer_ = 0.0f;

	// 0..1 にクランプ
	const float t = std::clamp(overall, 0.0f, 1.0f);

	// --- 2nd は常に同じクリップを使う前提（必要なら名前をJson化）
	if (player_->GetCurrentAnimationName() != "player_attack_2nd") {
		player_->SetNextAnimation("player_attack_2nd", /*loop*/false, /*blend*/0.0f);
	}
	const float dur = player_->GetAnimationDuration("player_attack_2nd");
	player_->SetCurrentAnimTime(dur * t);

	// --- 経路の逆算: overall → segmentIndex + local
	//     2nd は 3 分割（kNumSegments）
	constexpr float kEps = 1e-6f;
	const float s = std::min(t * static_cast<float>(kNumSegments), static_cast<float>(kNumSegments) - kEps);
	const size_t segIndex = static_cast<size_t>(std::floor(s));     // 0,1,2
	const float  segLocal = s - static_cast<float>(segIndex);        // 0..<1

	// easing を本来の Update と合わせる
	const float eased = EasedValue(attackPosEaseType_, std::clamp(segLocal, 0.0f, 1.0f));

	// --- 区間始点/終点
	// Enter 時に CalcWayPoints/CalcApproachWayPoints で wayPoints_ と startTranslation_ は確定している
	// （External時は PlayerStateController が Enter を一度だけ呼ぶ実装）
	Vector3 segStart = (segIndex == 0) ? startTranslation_ : wayPoints_[segIndex - 1];
	Vector3 segEnd = wayPoints_[segIndex];

	// 位置を反映
	const Vector3 pos = Vector3::Lerp(segStart, segEnd, eased);
	player_->SetTranslation(pos);
}

void PlayerAttack_2ndState::SetActionProgress() {

	ActionProgressMonitor* monitor = ActionProgressMonitor::GetInstance();
	int objectID = PlayerBaseAttackState::AddActionObject("PlayerAttack_2ndState");

	// 全体進捗
	monitor->AddOverall(objectID, "Attack Progress_2nd", [this]() -> float {
		float progress = 0.0f;
		if (player_->GetCurrentAnimationName() == "player_attack_2nd") {
			progress = player_->GetAnimationProgress();
		}
		return progress; });

	// 攻撃骨アニメーション
	monitor->AddSpan(objectID, "Skinned Animation",
		[]() { return 0.0f; },
		[]() { return 1.0f; },
		[this]() {
			float progress = 0.0f;
			if (player_->GetCurrentAnimationName() == "player_attack_2nd") {

				progress = player_->GetAnimationProgress();
			}
			return progress; });

	// 経路補間の全体内進捗
	monitor->AddSpan(objectID, "Move Path",
		[]() { return 0.0f; },
		[]() { return 1.0f; },
		[this]() {
			float segmentLocal = 0.0f;
			if (segmentTime_ > 0.0f) {
				segmentLocal = std::clamp(segmentTimer_ / segmentTime_, 0.0f, 1.0f);
			}
			float done = (currentIndex_ >= kNumSegments) ? kNumSegments : static_cast<float>(currentIndex_);
			float overall = (done + segmentLocal) / kNumSegments;
			return overall; });

	SetSpanUpdate(objectID);
}

void PlayerAttack_2ndState::SetSpanUpdate(int objectID) {

	ActionProgressMonitor* monitor = ActionProgressMonitor::GetInstance();

	// External ON/OFF → 基底で player_->SetUpdateMode(...) 切替 & externalActive_ を持つ
	PlayerBaseAttackState::SetSynchObject(objectID);

	// Overall 駆動（カメラ → 状態）
	monitor->SetOverallDriveHandler(objectID, [this](float overall) {
		DriveOverall(overall);
		});
}