#include "PlayerAttack_3rdState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	PlayerAttack_3rdState classMethods
//============================================================================

PlayerAttack_3rdState::PlayerAttack_3rdState() {

	// debug
	debugForward_.emplace(PlayerWeaponType::Left, Vector3::AnyInit(0.0f));
	debugForward_.emplace(PlayerWeaponType::Right, Vector3::AnyInit(0.0f));
}

void PlayerAttack_3rdState::Enter(Player& player) {

	player.SetNextAnimation("player_attack_3rd", false, nextAnimDuration_);
	canExit_ = false;

	// 敵が攻撃可能範囲にいるかチェック
	assisted_ = CheckInRange(attackPosLerpCircleRange_,
		Vector3(bossEnemy_->GetTranslation() - backStartPos_).Length());

	// 補間座標を設定
	backStartPos_ = player.GetTranslation();
	Vector3 direction = (bossEnemy_->GetTranslation() - backStartPos_).Normalize();
	// 補間先がいなければ正面向き
	if (!assisted_) {
		direction = player.GetTransform().GetForward();
	}
	backTargetPos_ = backStartPos_ + direction * backMoveValue_;
	currentState_ = State::MoveBack;

	// Y座標の固定値
	initPosY_ = player.GetTranslation().y;
}

void PlayerAttack_3rdState::Update(Player& player) {

	// animationが終わったかチェック
	canExit_ = player.IsAnimationFinished();
	// animationが終わったら時間経過を進める
	if (canExit_) {

		exitTimer_ += GameTimer::GetScaledDeltaTime();
	}

	// プレイヤーの補間処理
	LerpPlayer(player);

	// 座標、回転補間
	AttackAssist(player);
	// キーイベント処理
	UpdateAnimKeyEvent(player);

	// 武器補間処理
	LerpWeapon(player, PlayerWeaponType::Left);
	LerpWeapon(player, PlayerWeaponType::Right);
}

void PlayerAttack_3rdState::LerpWeapon(Player& player, PlayerWeaponType type) {

	if (currentState_ == State::None) {
		return;
	}

	// 補間開始合図まで処理しない
	if (!weaponParams_[type].isMoveStart) {
		return;
	}

	// 時間を進める
	weaponParams_[type].moveTimer.Update();

	// 補間座標を剣に設定
	Vector3 pos = Vector3::Lerp(weaponParams_[type].startPos,
		weaponParams_[type].targetPos, weaponParams_[type].moveTimer.easedT_);
	player.GetWeapon(type)->SetTranslation(pos);

	// 補間終了後座標をとどめる
	if (weaponParams_[type].moveTimer.IsReached()) {

		player.GetWeapon(type)->SetTranslation(weaponParams_[type].targetPos);
	}


	switch (type) {
	case PlayerWeaponType::Left:

		// 回転
		weaponParams_[type].rotation.z = pi / 2.0f;
		weaponParams_[type].rotation.y += weaponParams_[type].rotateSpeed;
		player.GetWeapon(type)->SetRotation(Quaternion::Normalize(
			Quaternion::EulerToQuaternion(weaponParams_[type].rotation)));
		break;
	case PlayerWeaponType::Right:

		// 回転
		weaponParams_[type].rotation.x = pi / 2.0f;
		weaponParams_[type].rotation.y += weaponParams_[type].rotateSpeed;
		player.GetWeapon(type)->SetRotation(Quaternion::Normalize(
			Quaternion::EulerToQuaternion(weaponParams_[type].rotation)));
		break;
	}
}

void PlayerAttack_3rdState::LerpPlayer(Player& player) {

	switch (currentState_) {
	case State::None: {
		break;
	}
	case PlayerAttack_3rdState::State::MoveBack: {

		// 時間を進める
		backMoveTimer_.Update();
		// 座標を補間
		Vector3 pos = Vector3::Lerp(backStartPos_, backTargetPos_,
			backMoveTimer_.easedT_);
		player.SetTranslation(pos);

		// 補間終了後座標をとどめる
		if (backMoveTimer_.IsReached()) {

			player.SetTranslation(backTargetPos_);
		}
		break;
	}
	case PlayerAttack_3rdState::State::Catch: {

		// 時間を進める
		catchSwordTimer_.Update();
		// 座標を補間
		Vector3 pos = Vector3::Lerp(backTargetPos_, catchTargetPos_,
			catchSwordTimer_.easedT_);
		player.SetTranslation(pos);

		// 補間終了後座標をとどめる
		if (catchSwordTimer_.IsReached()) {

			player.SetTranslation(catchTargetPos_);
			currentState_ = State::None;

			// 親子付けを元に戻す
			player.ResetWeaponTransform();
		}
		break;
	}
	}
}

void PlayerAttack_3rdState::UpdateAnimKeyEvent(Player& player) {

	// 左手を離した瞬間
	if (player.IsEventKey("OutSword", 0)) {

		// 補間開始
		StartMoveWeapon(player, PlayerWeaponType::Left);
		return;
	}

	// 右手を離した瞬間
	if (player.IsEventKey("OutSword", 1)) {

		// 補間開始
		StartMoveWeapon(player, PlayerWeaponType::Right);
		return;
	}

	// 剣を取りに行く
	if (player.IsEventKey("CatchSword", 0)) {

		// 剣を取りに行く状態に遷移
		currentState_ = State::Catch;

		// 目標座標を剣の間の座標に設定する
		catchTargetPos_ = (player.GetWeapon(PlayerWeaponType::Left)->GetTranslation() +
			player.GetWeapon(PlayerWeaponType::Right)->GetTranslation()) / 2.0f;
		catchTargetPos_.y = initPosY_;
	}
}

void PlayerAttack_3rdState::StartMoveWeapon(Player& player, PlayerWeaponType type) {

	// α値を下げる
	player.GetWeapon(type)->SetAlpha(0.5f);
	// 剣の親子付けを解除する
	player.GetWeapon(type)->SetParent(Transform3D(), true);
	// この時点のワールド座標を補間開始座標にする
	weaponParams_[type].startPos =
		player.GetWeapon(type)->GetTransform().GetWorldPos();
	weaponParams_[type].startPos.y = weaponPosY_;
	// 開始座標として設定しておく
	player.GetWeapon(type)->SetTranslation(
		weaponParams_[type].startPos);

	// 目標座標を設定する
	if (assisted_) {

		// 敵への向き
		Vector3 toEnemy = bossEnemy_->GetTranslation() - player.GetTranslation();
		toEnemy.y = 0.0f;
		toEnemy = toEnemy.Normalize();

		// Y軸回転オフセットをかける
		Vector3 rotated = RotateYOffset(toEnemy, weaponParams_[type].offsetRotationY);

		// 敵を中心に一定距離だけオフセット
		weaponParams_[type].targetPos = bossEnemy_->GetTranslation() + rotated * bossEnemyDistance_;
		weaponParams_[type].targetPos.y = weaponPosY_;

		// デバッグ用
		debugForward_[type] = rotated;
	} else {

		// 前方ベクトル
		Vector3 forward = Quaternion::RotateVector(Vector3(0.0f, 0.0f, 1.0f), player.GetRotation());
		forward.y = 0.0f;
		forward = forward.Normalize();

		// Y軸回転オフセットをかける
		Vector3 rotated = RotateYOffset(forward, weaponParams_[type].offsetRotationY / 6.0f);

		// 目標座標
		weaponParams_[type].targetPos = player.GetTranslation() + rotated * weaponParams_[type].moveValue;
		weaponParams_[type].targetPos.y = weaponPosY_;

		// デバッグ用
		debugForward_[type] = rotated;
	}

	// 補間開始
	weaponParams_[type].isMoveStart = true;
	weaponParams_[type].moveTimer = weaponMoveTimer_;
}

Vector3 PlayerAttack_3rdState::RotateYOffset(const Vector3& direction, float offsetRotationY) {

	float cos = std::cos(offsetRotationY);
	float sin = std::sin(offsetRotationY);

	Vector3 rotated{};
	rotated.x = direction.x * cos - direction.z * sin;
	rotated.y = 0.0f;
	rotated.z = direction.x * sin + direction.z * cos;

	return rotated;
}

void PlayerAttack_3rdState::Exit(Player& player) {

	// リセット
	attackPosLerpTimer_ = 0.0f;
	exitTimer_ = 0.0f;

	backMoveTimer_.Reset();
	catchSwordTimer_.Reset();
	for (auto& [type, param] : weaponParams_) {

		param.isMoveStart = false;
		param.moveTimer.Reset();
		param.rotation.Init();
	}

	// 剣の親子付けを戻す
	if (currentState_ != State::None) {

		player.ResetWeaponTransform();
	}
}

void PlayerAttack_3rdState::ImGui(const Player& player) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);
	ImGui::DragFloat("bossEnemyDistance", &bossEnemyDistance_, 0.01f);
	ImGui::DragFloat("weaponPosY", &weaponPosY_, 0.01f);

	PlayerBaseAttackState::ImGui(player);

	ImGui::Separator();

	backMoveTimer_.ImGui("BackMoveTimer");
	ImGui::DragFloat("backMoveValue", &backMoveValue_, 0.1f);

	catchSwordTimer_.ImGui("CatchMoveTimer");

	weaponMoveTimer_.ImGui("WeaponMoveTimer");

	for (auto& [type, param] : weaponParams_) {

		ImGui::PushID(static_cast<uint32_t>(type));

		ImGui::SeparatorText(EnumAdapter<PlayerWeaponType>::ToString(type));

		ImGui::Text(std::format("isMoveStart: {}", param.isMoveStart).c_str());
		ImGui::DragFloat("moveValue", &param.moveValue, 0.1f);
		ImGui::DragFloat("rotateSpeed", &param.rotateSpeed, 0.01f);
		ImGui::DragFloat("offsetRotationY", &param.offsetRotationY, 0.01f);
		ImGui::DragFloat3("forward", &debugForward_[type].x);

		ImGui::PopID();
	}
}

void PlayerAttack_3rdState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	exitTime_ = JsonAdapter::GetValue<float>(data, "exitTime_");
	bossEnemyDistance_ = JsonAdapter::GetValue<float>(data, "bossEnemyDistance_");
	weaponPosY_ = JsonAdapter::GetValue<float>(data, "weaponPosY_");

	PlayerBaseAttackState::ApplyJson(data);

	backMoveTimer_.FromJson(data["BackMoveTimer"]);
	catchSwordTimer_.FromJson(data["CatchMoveTimer"]);
	weaponMoveTimer_.FromJson(data["WeaponMoveTimer"]);

	backMoveValue_ = data.value("backMoveValue_", 1.0f);

	weaponParams_.emplace(PlayerWeaponType::Left, WeaponParam());
	weaponParams_.emplace(PlayerWeaponType::Right, WeaponParam());
	if (data.contains("Weapon")) {
		for (auto& [type, param] : weaponParams_) {

			const auto& key = EnumAdapter<PlayerWeaponType>::ToString(type);
			param.moveValue = data["Weapon"][key]["moveValue"];
			param.rotateSpeed = data["Weapon"][key].value("rotateSpeed", 1.0f);
			param.offsetRotationY = data["Weapon"][key]["offsetRotationY"];
		}
	}
}

void PlayerAttack_3rdState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["exitTime_"] = exitTime_;
	data["bossEnemyDistance_"] = bossEnemyDistance_;
	data["weaponPosY_"] = weaponPosY_;

	PlayerBaseAttackState::SaveJson(data);

	backMoveTimer_.ToJson(data["BackMoveTimer"]);
	catchSwordTimer_.ToJson(data["CatchMoveTimer"]);
	weaponMoveTimer_.ToJson(data["WeaponMoveTimer"]);

	data["backMoveValue_"] = backMoveValue_;

	for (auto& [type, param] : weaponParams_) {

		const auto& key = EnumAdapter<PlayerWeaponType>::ToString(type);
		data["Weapon"][key]["moveValue"] = param.moveValue;
		data["Weapon"][key]["rotateSpeed"] = param.rotateSpeed;
		data["Weapon"][key]["offsetRotationY"] = param.offsetRotationY;
	}
}

bool PlayerAttack_3rdState::GetCanExit() const {

	// 経過時間が過ぎたら
	bool canExit = exitTimer_ > exitTime_;
	return canExit;
}