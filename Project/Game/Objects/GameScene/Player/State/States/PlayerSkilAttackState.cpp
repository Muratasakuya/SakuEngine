#include "PlayerSkilAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/ActionProgress/ActionProgressMonitor.h>
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	PlayerSkilAttackState classMethods
//============================================================================

PlayerSkilAttackState::PlayerSkilAttackState(Player* player) {

	player_ = nullptr;
	player_ = player;

	rushMoveParam_.name = "RushMove";
	returnMoveParam_.name = "ReturnMove";
	backJumpParam_.name = "BackJump";
	jumpMoveParam_.name = "MoveJump";

	rushMoveParam_.timer.Reset();
	returnMoveParam_.timer.Reset();
	backJumpParam_.timer.Reset();
	jumpMoveParam_.timer.Reset();
	canExit_ = false;
	isStratLookEnemy_ = false;
}

void PlayerSkilAttackState::Enter(Player& player) {

	// 初期状態を設定
	currentState_ = State::Rush;
	jumpAttackState_ = JumpAttackState::Jump;

	// 敵が攻撃可能範囲にいるかチェック
	const Vector3 playerPos = player.GetTranslation();
	const Vector3 bossEnemyPos = bossEnemy_->GetTranslation();
	Vector3 direction = bossEnemyPos - playerPos;
	assisted_ = CheckInRange(attackPosLerpCircleRange_,
		Vector3(direction).Length());

	// 範囲内にいない場合はスキル攻撃を出来ない
	if (assisted_) {

		// 最初の補間座標を選択
		rushMoveParam_.start = playerPos;
		rushMoveParam_.start.y = 0.0f;
		direction = direction.Normalize();
		rushMoveParam_.target = bossEnemyPos + direction * rushMoveParam_.moveValue;
		rushMoveParam_.target.y = 0.0f;
		// アニメーションを設定
		player.SetNextAnimation("player_skilAttack_1st", false, rushMoveParam_.nextAnim);
		// 敵の方を向ける
		player.SetRotation(Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f)));
	} else {

		canExit_ = true;
		// 確実に終了するようにする
		exitTimer_ = exitTime_ * 2.0f;
	}
}

void PlayerSkilAttackState::Update(Player& player) {

	if (!assisted_ || canExit_) {
		return;
	}

	// 各状態を更新
	UpdateState(player);
}

void PlayerSkilAttackState::UpdateState(Player& player) {

	switch (currentState_) {
	case PlayerSkilAttackState::State::Rush:

		UpdateRush(player);
		break;
	case PlayerSkilAttackState::State::Return:

		UpdateReturn(player);
		break;
	case PlayerSkilAttackState::State::JumpAttack:

		UpdateJumpAttack(player);
		break;
	}
}

void PlayerSkilAttackState::UpdateRush(Player& player) {

	// 座標を補間する
	PlayerBaseAttackState::UpdateTimer(rushMoveParam_.timer);
	player.SetTranslation(Vector3::Lerp(rushMoveParam_.start,
		rushMoveParam_.target, rushMoveParam_.timer.easedT_));

	// 終了後次の状態に進める
	if (rushMoveParam_.timer.IsReached()) {

		const Vector3 playerPos = player.GetTranslation();
		const Vector3 bossEnemyPos = bossEnemy_->GetTranslation();
		Vector3 direction = Vector3(bossEnemyPos - playerPos).Normalize();

		// 次の補間座標を設定
		returnMoveParam_.start = rushMoveParam_.target;
		returnMoveParam_.start.y = 0.0f;
		returnMoveParam_.target = bossEnemyPos + direction * returnMoveParam_.moveValue;
		returnMoveParam_.target.y = 0.0f;

		// アニメーションを設定
		player.SetNextAnimation("player_skilAttack_2nd", false, returnMoveParam_.nextAnim);
		// 敵の方を向ける
		player.SetRotation(Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f)));

		// 次に進める
		currentState_ = State::Return;
	}
}

void PlayerSkilAttackState::UpdateReturn(Player& player) {

	// 座標を補間する
	PlayerBaseAttackState::UpdateTimer(returnMoveParam_.timer);
	player.SetTranslation(Vector3::Lerp(returnMoveParam_.start,
		returnMoveParam_.target, returnMoveParam_.timer.easedT_));

	// 回転補間
	// 一定の経過率後回転補間を開始する
	if (lookStartProgress_ < returnMoveParam_.timer.t_ &&
		!isStratLookEnemy_) {

		// 現在の回転、位置から敵への回転補間を設定
		yawLerpStart_ = player.GetRotation();
		const Vector3 playerPos = player.GetTranslation();
		const Vector3 bossEnemyPos = bossEnemy_->GetTranslation();
		Vector3 directionXZ = Vector3(bossEnemyPos.x - playerPos.x, 0.0f, bossEnemyPos.z - playerPos.z).Normalize();
		yawLerpEnd_ = Quaternion::LookRotation(directionXZ, Vector3(0.0f, 1.0f, 0.0f));

		isStratLookEnemy_ = true;
	}
	if (isStratLookEnemy_) {

		// 敵方向へ補間
		lookEnemyTimer_.Update();
		player.SetRotation(Quaternion::Slerp(yawLerpStart_, yawLerpEnd_, lookEnemyTimer_.easedT_));
	}

	// 終了後次の状態に進める
	if (returnMoveParam_.timer.IsReached()) {

		const Vector3 playerPos = player.GetTranslation();
		const Vector3 bossEnemyPos = bossEnemy_->GetTranslation();
		Vector3 direction = Vector3(bossEnemyPos - playerPos).Normalize();

		// 次の補間座標を設定
		backJumpParam_.start = returnMoveParam_.target;
		backJumpParam_.target = playerPos + direction * backJumpParam_.moveValue;
		// ジャンプ力を設定
		backJumpParam_.target.y = jumpStrength_;

		// アニメーションを設定
		player.SetNextAnimation("player_skilAttack_3rd", false, backJumpParam_.nextAnim);
		// 敵の方を向ける
		player.SetRotation(Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f)));

		// 次に進める
		currentState_ = State::JumpAttack;
	}
}

void PlayerSkilAttackState::UpdateJumpAttack(Player& player) {

	switch (jumpAttackState_) {
	case PlayerSkilAttackState::JumpAttackState::Jump: {

		// 座標を補間する
		PlayerBaseAttackState::UpdateTimer(backJumpParam_.timer);
		player.SetTranslation(Vector3::Lerp(backJumpParam_.start,
			backJumpParam_.target, backJumpParam_.timer.easedT_));

		// 終了後次の状態に移す
		if (backJumpParam_.timer.IsReached()) {

			const Vector3 playerPos = player.GetTranslation();
			const Vector3 bossEnemyPos = bossEnemy_->GetTranslation();
			Vector3 direction = Vector3(bossEnemyPos - playerPos).Normalize();

			// 次の補間座標を設定
			jumpMoveParam_.start = backJumpParam_.target;
			jumpMoveParam_.target = bossEnemyPos + direction * jumpMoveParam_.moveValue;
			jumpMoveParam_.target.y = 0.0f;

			// 敵の方を向ける
			player.SetRotation(Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f)));

			// 次に進める
			jumpAttackState_ = JumpAttackState::Attack;
		}
		break;
	}
	case PlayerSkilAttackState::JumpAttackState::Attack: {

		// 座標を補間する
		PlayerBaseAttackState::UpdateTimer(jumpMoveParam_.timer);
		player.SetTranslation(Vector3::Lerp(jumpMoveParam_.start,
			jumpMoveParam_.target, jumpMoveParam_.timer.easedT_));

		// 補間終了後状態を閉じる
		if (jumpMoveParam_.timer.IsReached()) {

			canExit_ = true;
			// 確実に終了するようにする
			exitTimer_ = exitTime_ * 2.0f;
		}
		break;
	}
	}
}

void PlayerSkilAttackState::Exit(Player& player) {

	// リセット
	canExit_ = false;
	isStratLookEnemy_ = false;
	exitTimer_ = 0.0f;
	rushMoveParam_.timer.Reset();
	returnMoveParam_.timer.Reset();
	backJumpParam_.timer.Reset();
	jumpMoveParam_.timer.Reset();
	lookEnemyTimer_.Reset();
	if (assisted_) {

		player.ResetAnimation();
	}
}

void PlayerSkilAttackState::ImGui(const Player& player) {

	ImGui::Text("currentState: %s", EnumAdapter<State>::ToString(currentState_));
	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::Text(std::format("assisted: {}", assisted_).c_str());

	ImGui::Separator();

	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);

	if (ImGui::CollapsingHeader("Base Attack")) {

		PlayerBaseAttackState::ImGui(player);
	}
	EnumAdapter<State>::Combo("State", &editState_);
	switch (editState_) {
	case PlayerSkilAttackState::State::Rush:

		rushMoveParam_.ImGui(player, *bossEnemy_, true);
		break;
	case PlayerSkilAttackState::State::Return:

		ImGui::DragFloat("lookStartProgress", &lookStartProgress_, 0.01f);

		ImGui::SeparatorText("Move");

		returnMoveParam_.ImGui(player, *bossEnemy_, true);

		lookEnemyTimer_.ImGui("LookEnemy");
		break;
	case PlayerSkilAttackState::State::JumpAttack:

		backJumpParam_.ImGui(player, *bossEnemy_, false);
		ImGui::DragFloat("jumpStrength", &jumpStrength_, 0.1f);
		ImGui::Separator();
		jumpMoveParam_.ImGui(player, *bossEnemy_, true);
		break;
	}
}

void PlayerSkilAttackState::StateMoveParam::ImGui(
	const Player& player, const BossEnemy& bossEnemy, bool isBaseEnemy) {

	ImGui::PushID(name.c_str());

	timer.ImGui("MoveTimer");
	ImGui::DragFloat("moveValue", &moveValue, 0.01f);
	ImGui::DragFloat("nextAnim", &nextAnim, 0.01f);
	ImGui::ProgressBar(timer.t_);

	LineRenderer* renderer = LineRenderer::GetInstance();

	Vector3 startPos = player.GetTranslation();
	const Vector3 bossEnemyPos = bossEnemy.GetTranslation();
	Vector3 direction = Vector3(bossEnemyPos - startPos).Normalize();
	Vector3 basePos = startPos;
	if (isBaseEnemy) {

		basePos = bossEnemyPos;
	}
	Vector3 targetPos = basePos + direction * moveValue;
	startPos.y = 2.0f;
	targetPos.y = 2.0f;
	renderer->DrawLine3D(startPos, targetPos, Color::Cyan());
	renderer->DrawSphere(8, 4.0f, targetPos, Color::Cyan());

	ImGui::PopID();
}

void PlayerSkilAttackState::JumpParam::ImGui() {

	ImGui::DragFloat("power", &power, 0.01f);
	ImGui::DragFloat("gravity", &gravity, 0.01f);
}

void PlayerSkilAttackState::StateMoveParam::ApplyJson(const Json& data) {

	if (!data.contains(name)) {
		return;
	}

	timer.FromJson(data[name]);
	moveValue = data[name].value("moveValue", 32.0f);
	nextAnim = data[name].value("nextAnim", 0.01f);
}

void PlayerSkilAttackState::StateMoveParam::SaveJson(Json& data) {

	timer.ToJson(data[name]);
	data[name]["moveValue"] = moveValue;
	data[name]["nextAnim"] = nextAnim;
}

void PlayerSkilAttackState::JumpParam::ApplyJson(const Json& data) {

	if (!data.contains(name)) {
		return;
	}

	power = data[name].value("power", 1.0f);
	gravity = data[name].value("gravity", 1.0f);
}

void PlayerSkilAttackState::JumpParam::SaveJson(Json& data) {

	data[name]["power"] = power;
	data[name]["gravity"] = gravity;
}

void PlayerSkilAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	exitTime_ = JsonAdapter::GetValue<float>(data, "exitTime_");

	PlayerBaseAttackState::ApplyJson(data);

	rushMoveParam_.ApplyJson(data);
	returnMoveParam_.ApplyJson(data);
	backJumpParam_.ApplyJson(data);
	jumpMoveParam_.ApplyJson(data);

	lookEnemyTimer_.FromJson(data.value("LookEnemy", Json()));
	lookStartProgress_ = data.value("lookStartProgress_", 0.8f);
	jumpStrength_ = data.value("jumpStrength_", 0.8f);

	SetActionProgress();
}

void PlayerSkilAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["exitTime_"] = exitTime_;

	PlayerBaseAttackState::SaveJson(data);

	rushMoveParam_.SaveJson(data);
	returnMoveParam_.SaveJson(data);
	backJumpParam_.SaveJson(data);
	jumpMoveParam_.SaveJson(data);

	lookEnemyTimer_.ToJson(data["LookEnemy"]);
	data["lookStartProgress_"] = lookStartProgress_;
	data["jumpStrength_"] = jumpStrength_;
}

bool PlayerSkilAttackState::GetCanExit() const {

	// 経過時間が過ぎたら
	bool canExit = exitTimer_ > exitTime_;
	return canExit;
}

void PlayerSkilAttackState::DriveOverall(float overall) {

	// 総時間
	const float t1 = std::max(0.0f, rushMoveParam_.timer.target_);
	const float t2 = std::max(0.0f, returnMoveParam_.timer.target_);
	const float t3 = std::max(0.0f, backJumpParam_.timer.target_);
	const float t4 = std::max(0.0f, jumpMoveParam_.timer.target_);
	const float total = std::max(1e-6f, t1 + t2 + t3 + t4);

	const float c1 = t1 / total;
	const float c2 = (t1 + t2) / total;
	const float c3 = (t1 + t2 + t3) / total;

	// 全体進捗に応じて更新する状態を切り替える

	// Rush
	if (Algorithm::InRangeOverall(overall, 0.0f, c1)) {
		currentState_ = State::Rush;
		SetTimerByOverall(rushMoveParam_.timer, overall, 0.0f, c1, rushMoveParam_.timer.easeingType_);

		// アニメーションを切り替える
		player_->SetNextAnimation("player_skilAttack_1st", false, 0.0f);
		isStratLookEnemy_ = false;
		lookEnemyTimer_.Reset();
		return;
	}

	// Return
	if (Algorithm::InRangeOverall(overall, c1, c2)) {

		currentState_ = State::Return;
		SetTimerByOverall(returnMoveParam_.timer, overall, c1, c2, returnMoveParam_.timer.easeingType_);

		// アニメーションを切り替える
		player_->SetNextAnimation("player_skilAttack_2nd", false, 0.0f);

		const float local = Algorithm::MapOverallToLocal(overall, c1, c2);
		// 敵向き補間
		if (local >= lookStartProgress_) {

			isStratLookEnemy_ = true;
			const float lookLocal = Algorithm::MapOverallToLocal(local, lookStartProgress_, 1.0f);
			lookEnemyTimer_.t_ = lookLocal;
			lookEnemyTimer_.easedT_ = EasedValue(lookEnemyTimer_.easeingType_, lookLocal);
		} else {

			isStratLookEnemy_ = false;
			lookEnemyTimer_.Reset();
		}
		return;
	}

	// BackJump
	if (Algorithm::InRangeOverall(overall, c2, c3)) {

		currentState_ = State::JumpAttack;
		jumpAttackState_ = JumpAttackState::Jump;
		SetTimerByOverall(backJumpParam_.timer, overall, c2, c3, backJumpParam_.timer.easeingType_);

		// アニメーションを設定
		player_->SetNextAnimation("player_skilAttack_3rd", false, 0.0f);
		return;
	}

	// JumpMove
	if (Algorithm::InRangeOverall(overall, c3, 1.0f)) {

		currentState_ = State::JumpAttack;
		jumpAttackState_ = JumpAttackState::Attack;
		SetTimerByOverall(jumpMoveParam_.timer, overall, c3, 1.0f, jumpMoveParam_.timer.easeingType_);
		return;
	}
}

void PlayerSkilAttackState::SetActionProgress() {

	ActionProgressMonitor* monitor = ActionProgressMonitor::GetInstance();
	int objectID = PlayerBaseAttackState::AddActionObject("PlayerSkilAttackState");

	// それぞれの処理の目標時間
	const float target1 = (std::max)(0.0f, rushMoveParam_.timer.target_);
	const float target2 = (std::max)(0.0f, returnMoveParam_.timer.target_);
	const float target3 = (std::max)(0.0f, backJumpParam_.timer.target_);
	const float target4 = (std::max)(0.0f, jumpMoveParam_.timer.target_);
	const float totalT = (std::max)(1e-6f, target1 + target2 + target3 + target4);

	// 全体進捗
	monitor->AddOverall(objectID, "Attack Progress", [=, this]() -> float {

		float t = 0.0f;
		t += rushMoveParam_.timer.t_;
		if (rushMoveParam_.timer.IsReached()) {

			t = target1 + returnMoveParam_.timer.t_;
		}
		if (returnMoveParam_.timer.IsReached()) {

			t = target1 + target2 + backJumpParam_.timer.t_;
		}
		if (backJumpParam_.timer.IsReached()) {

			t = target1 + target2 + target3 + jumpMoveParam_.timer.t_;
		}
		return std::clamp(t / totalT, 0.0f, 1.0f); });

	// Rush
	monitor->AddSpan(objectID, "Rush Move",
		[=]() { return 0.0f; },
		[=]() { return std::clamp(target1 / totalT, 0.0f, 1.0f); },
		[this]() { return std::clamp(rushMoveParam_.timer.easedT_, 0.0f, 1.0f); });

	// Return
	monitor->AddSpan(objectID, "Return Move",
		[=]() { return std::clamp(target1 / totalT, 0.0f, 1.0f); },
		[=]() { return std::clamp((target1 + target2) / totalT, 0.0f, 1.0f); },
		[this]() { return std::clamp(returnMoveParam_.timer.easedT_, 0.0f, 1.0f); });

	// Back Jump
	monitor->AddSpan(objectID, "Back Jump",
		[=]() { return std::clamp((target1 + target2) / totalT, 0.0f, 1.0f); },
		[=]() { return std::clamp((target1 + target2 + target3) / totalT, 0.0f, 1.0f); },
		[this]() { return std::clamp(backJumpParam_.timer.easedT_, 0.0f, 1.0f); });

	// Jump Move
	monitor->AddSpan(objectID, "Jump Move",
		[=]() { return std::clamp((target1 + target2 + target3) / totalT, 0.0f, 1.0f); },
		[=]() { return 1.0f; },
		[this]() { return std::clamp(jumpMoveParam_.timer.easedT_, 0.0f, 1.0f); });

	// 進捗率の同期設定
	SetSpanUpdate(objectID);
}

void PlayerSkilAttackState::SetSpanUpdate(int objectID) {
	
	ActionProgressMonitor* monitor = ActionProgressMonitor::GetInstance();

	// 同期設定
	PlayerBaseAttackState::SetSynchObject(objectID);

	// 全体進捗による同期
	monitor->SetOverallDriveHandler(objectID, [this](float overall) {
		DriveOverall(overall); });
}