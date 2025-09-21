#include "PlayerSkilAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/GameTimer.h>
#include <Engine/Utility/EnumAdapter.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	PlayerSkilAttackState classMethods
//============================================================================

PlayerSkilAttackState::PlayerSkilAttackState() {

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
	rushMoveParam_.timer.Update();
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
	returnMoveParam_.timer.Update();
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
		backJumpParam_.timer.Update();
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
		jumpMoveParam_.timer.Update();
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