#include "PlayerAttack_3rdState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/ActionProgress/ActionProgressMonitor.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	PlayerAttack_3rdState classMethods
//============================================================================

PlayerAttack_3rdState::PlayerAttack_3rdState(Player* player) {

	player_ = nullptr;
	player_ = player;

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

	// 進捗更新
	totalTimer_.Update();
}

void PlayerAttack_3rdState::LerpWeapon(Player& player, PlayerWeaponType type) {

	// 補間開始合図まで処理しない
	if (!weaponParams_[type].isMoveStart) {
		return;
	}

	// 時間を進める
	PlayerBaseAttackState::UpdateTimer(weaponParams_[type].moveTimer);

	// 補間座標を剣に設定
	Vector3 pos = Vector3::Lerp(weaponParams_[type].startPos,
		weaponParams_[type].targetPos, weaponParams_[type].moveTimer.easedT_);
	player.GetWeapon(type)->SetTranslation(pos);

	// 補間終了後座標をとどめる
	if (weaponParams_[type].moveTimer.IsReached()) {

		player.GetWeapon(type)->SetTranslation(weaponParams_[type].targetPos);
	}

	// 剣を取り終えたら回転しない
	if (catchSwordTimer_.IsReached()) {
		return;
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
		PlayerBaseAttackState::UpdateTimer(backMoveTimer_);
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
		PlayerBaseAttackState::UpdateTimer(catchSwordTimer_);
		// 座標を補間
		Vector3 pos = Vector3::Lerp(backTargetPos_, catchTargetPos_,
			catchSwordTimer_.easedT_);
		player.SetTranslation(pos);

		// 補間終了後座標をとどめる
		if (catchSwordTimer_.IsReached()) {

			player.SetTranslation(catchTargetPos_);
			currentState_ = State::None;

			// 親子付けを元に戻す
			player.ResetWeaponTransform(PlayerWeaponType::Left);
			player.ResetWeaponTransform(PlayerWeaponType::Right);
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

	float duration = player.GetAnimationDuration("player_attack_3rd");
	float currentProgress = player.GetAnimationProgress();
	float spanLength = weaponMoveTimer_.target_ / duration;

	weaponParams_[type].startProgress = std::clamp(currentProgress, 0.0f, 1.0f);
	weaponParams_[type].endProgress = std::clamp(currentProgress + spanLength, 0.0f, 1.0f);
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
	totalTimer_.Reset();
	for (auto& [type, param] : weaponParams_) {

		param.isMoveStart = false;
		param.moveTimer.Reset();
		param.rotation.Init();
	}

	// 剣の親子付けを戻す
	if (currentState_ != State::None) {

		player.ResetWeaponTransform(PlayerWeaponType::Left);
		player.ResetWeaponTransform(PlayerWeaponType::Right);
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

	// 合計処理時間を設定
	totalTimer_.target_ = player_->GetEventTime(
		"player_attack_3rd", "CatchSword", 0) + catchSwordTimer_.target_;

	SetActionProgress();
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

void PlayerAttack_3rdState::DriveOverall(float overall) {

	const float totalT = std::max(1e-6f, totalTimer_.target_);

	const float backEnd = std::max(0.0f, backMoveTimer_.target_) / totalT;
	const float catchBegin = std::max(0.0f, player_->GetEventTime("player_attack_3rd", "CatchSword", 0)) / totalT;
	const float catchEnd = std::max(catchBegin, catchBegin + std::max(0.0f, catchSwordTimer_.target_ / totalT));

	const float outLbegin = std::max(0.0f, player_->GetEventTime("player_attack_3rd", "OutSword", 0)) / totalT;
	const float outRbegin = std::max(0.0f, player_->GetEventTime("player_attack_3rd", "OutSword", 1)) / totalT;
	const float outLend = std::max(outLbegin, outLbegin + std::max(0.0f, weaponMoveTimer_.target_ / totalT));
	const float outRend = std::max(outRbegin, outRbegin + std::max(0.0f, weaponMoveTimer_.target_ / totalT));

	if (catchEnd - 1e-6f <= overall) {
		if (!endAttached_) {

			// 右手はこの瞬間に親子付けを戻す
			EnsureWeaponReset(PlayerWeaponType::Left);
			EnsureWeaponReset(PlayerWeaponType::Right);
			endAttached_ = true;
		}
		currentState_ = State::None;
		return;
	} else {
		if (endAttached_) {

			endAttached_ = false;
			EnsureWeaponHoldAtTargetDetached(PlayerWeaponType::Left);
			EnsureWeaponHoldAtTargetDetached(PlayerWeaponType::Right);
		}
	}

	// 全体進捗に応じて更新する状態を切り替える

	// プレイヤー移動
	if (Algorithm::InRangeOverall(overall, 0.0f, backEnd)) {

		currentState_ = State::MoveBack;
		SetTimerByOverall(backMoveTimer_, overall, 0.0f, backEnd, backMoveTimer_.easeingType_);
	} else if (Algorithm::InRangeOverall(overall, catchBegin, catchEnd)) {

		// 補間先を設定する
		if (currentState_ != State::Catch) {

			Vector3 left = player_->GetWeapon(PlayerWeaponType::Left)->GetTransform().GetWorldPos();
			Vector3 right = player_->GetWeapon(PlayerWeaponType::Right)->GetTransform().GetWorldPos();
			catchTargetPos_ = (left + right) * 0.5f;
			catchTargetPos_.y = initPosY_;
		}
		currentState_ = State::Catch;
		SetTimerByOverall(catchSwordTimer_, overall, catchBegin, catchEnd, catchSwordTimer_.easeingType_);
	} else {

		currentState_ = State::None;
	}

	// 武器投げ移動処理
	// 左
	if (Algorithm::InRangeOverall(overall, outLbegin, outLend)) {

		EnsureWeaponStarted(PlayerWeaponType::Left);
		SetTimerByOverall(weaponParams_.at(PlayerWeaponType::Left).moveTimer,
			overall, outLbegin, outLend, weaponMoveTimer_.easeingType_);
	} else {
		if (overall < outLbegin) {

			EnsureWeaponReset(PlayerWeaponType::Left);
		}
	}
	// 右
	if (Algorithm::InRangeOverall(overall, outRbegin, outRend)) {

		EnsureWeaponStarted(PlayerWeaponType::Right);
		SetTimerByOverall(weaponParams_.at(PlayerWeaponType::Right).moveTimer,
			overall, outRbegin, outRend, weaponMoveTimer_.easeingType_);
	} else {
		if (overall < outRbegin) {

			EnsureWeaponReset(PlayerWeaponType::Right);
		}
	}
}

void PlayerAttack_3rdState::EnsureWeaponStarted(PlayerWeaponType type) {

	// まだ剣がプレイヤーの手から離れていなければ
	if (!weaponParams_[type].isMoveStart) {

		// 手から離して補間を開始する
		StartMoveWeapon(*player_, type);
	}
}

void PlayerAttack_3rdState::EnsureWeaponReset(PlayerWeaponType type) {

	auto& param = weaponParams_[type];
	// 補間中
	if (param.isMoveStart) {

		// 補間を終了して元の位置に戻す
		param.isMoveStart = false;
		param.moveTimer.Reset();
		param.rotation.Init();
		player_->ResetWeaponTransform(type);
		player_->GetWeapon(type)->SetAlpha(1.0f);
	}
}

void PlayerAttack_3rdState::EnsureWeaponHoldAtTargetDetached(PlayerWeaponType type) {

	// すでに空中保持中なら何もしない
	auto& param = weaponParams_[type];
	if (param.isMoveStart && param.moveTimer.t_ >= 1.0f) {
		return;
	}
	// 親子付けを解除する
	player_->GetWeapon(type)->SetParent(Transform3D(), true);

	// 目標座標を保持
	Vector3 hold = param.targetPos;
	hold.y = weaponPosY_;
	player_->GetWeapon(type)->SetTranslation(hold);

	// 表示状態を補間中に戻す
	player_->GetWeapon(type)->SetAlpha(0.5f);
	param.isMoveStart = true;
	param.moveTimer.t_ = 1.0f;
	param.moveTimer.easedT_ = 1.0f;
}

void PlayerAttack_3rdState::SetActionProgress() {

	ActionProgressMonitor* monitor = ActionProgressMonitor::GetInstance();
	int objectID = PlayerBaseAttackState::AddActionObject("PlayerAttack_3rdState");

	// 全体進捗
	monitor->AddOverall(objectID, "Attack Progress", [this]() -> float {
		return std::clamp(totalTimer_.t_ / totalTimer_.target_, 0.0f, 1.0f); });

	// 0除算回避
	const float totalT = (std::max)(1e-6f, totalTimer_.target_);

	// プレイヤー移動
	const float backEndT = (std::max)(0.0f, backMoveTimer_.target_);
	const float catchStartT = (std::max)(0.0f, player_->GetEventTime("player_attack_3rd", "CatchSword", 0));
	const float catchEndT = (std::max)(catchStartT, catchStartT + (std::max)(0.0f, catchSwordTimer_.target_));
	// 武器移動
	const float outLeftT = (std::max)(0.0f, player_->GetEventTime("player_attack_3rd", "OutSword", 0));
	const float outRightT = (std::max)(0.0f, player_->GetEventTime("player_attack_3rd", "OutSword", 1));
	const float outLeftTEnd = (std::max)(outLeftT, outLeftT + (std::max)(0.0f, weaponMoveTimer_.target_));
	const float outRightTEnd = (std::max)(outRightT, outRightT + (std::max)(0.0f, weaponMoveTimer_.target_));

	// 骨アニメーション
	monitor->AddSpan(objectID, "Skinned Animation",
		[]() { return 0.0f; },
		[]() { return 1.0f; },
		[this]() {
			float progress = 0.0f;
			if (player_->GetCurrentAnimationName() == "player_attack_3rd") {

				progress = player_->GetAnimationProgress();
			}
			return progress; });

	// 後ずさり移動
	monitor->AddSpan(objectID, "Back Move",
		[=]() { return std::clamp(backEndT / totalT > 0 ? 0.0f : 0.0f, 0.0f, 1.0f); },
		[=]() { return std::clamp(backEndT / totalT, 0.0f, 1.0f); },
		[this]() { return backMoveTimer_.t_; });
	// 剣を取りに行く
	monitor->AddSpan(objectID, "Catch Move",
		[=]() { return std::clamp(catchStartT / totalT, 0.0f, 1.0f); },
		[=]() { return std::clamp(catchEndT / totalT, 0.0f, 1.0f); },
		[this]() { return catchSwordTimer_.t_; });

	// 剣投げ
	monitor->AddSpan(objectID, "LeftWeapon Move",
		[=]() { return std::clamp(outLeftT / totalT, 0.0f, 1.0f); },
		[=]() { return std::clamp(outLeftTEnd / totalT, 0.0f, 1.0f); },
		[this]() { return weaponParams_.at(PlayerWeaponType::Left).moveTimer.t_; });
	monitor->AddSpan(objectID, "RightWeapon Move",
		[=]() { return std::clamp(outRightT / totalT, 0.0f, 1.0f); },
		[=]() { return std::clamp(outRightTEnd / totalT, 0.0f, 1.0f); },
		[this]() { return weaponParams_.at(PlayerWeaponType::Right).moveTimer.t_; });

	// 進捗率の同期設定
	SetSpanUpdate(objectID);
}

void PlayerAttack_3rdState::SetSpanUpdate(int objectID) {

	ActionProgressMonitor* monitor = ActionProgressMonitor::GetInstance();

	// 同期設定
	PlayerBaseAttackState::SetSynchObject(objectID);

	// 攻撃骨アニメーション
	monitor->SetSpanSetter(objectID, "Skinned Animation", [this](float t) {

		// アニメーションを切り替え
		if (player_->GetCurrentAnimationName() != "player_attack_3rd") {

			player_->SetNextAnimation("player_attack_3rd", false, 0.0f);
		}

		const float duration = player_->GetAnimationDuration("player_attack_3rd");
		// アニメーションの時間を設定
		player_->SetCurrentAnimTime(duration * t);
		});

	// 全体進捗による同期
	monitor->SetOverallDriveHandler(objectID, [this](float overall) {
		DriveOverall(overall); });
}