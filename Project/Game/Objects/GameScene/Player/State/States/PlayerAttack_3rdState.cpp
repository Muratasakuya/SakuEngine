#include "PlayerAttack_3rdState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
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

	// ダッシュエフェクト作成
	catchDashEffect_ = std::make_unique<EffectGroup>();
	catchDashEffect_->Init("catchDashEffect", "PlayerEffect");
	catchDashEffect_->LoadJson("GameEffectGroup/Player/playerCatchDashEffect.json");

	// 残像表現エフェクト作成
	afterImageEffect_ = std::make_unique<PlayerAfterImageEffect>();
	afterImageEffect_->Init("playerAttack3rdDash");
}

void PlayerAttack_3rdState::Enter(Player& player) {

	player.SetNextAnimation("player_attack_3rd", false, nextAnimDuration_);
	canExit_ = false;

	// 補間座標を設定
	backStartPos_ = player.GetTranslation();

	// 敵が攻撃可能範囲にいるかチェック
	assisted_ = CheckInRange(attackPosLerpCircleRange_, PlayerIState::GetDistanceToBossEnemy());
	Vector3 direction = PlayerIState::GetDirectionToBossEnemy();
	// 補間先がいなければ正面向き
	if (!assisted_) {

		direction = player.GetTransform().GetForward();
		direction.y = 0.0f;
		direction = direction.Normalize();
	}
	backTargetPos_ = backStartPos_ + direction * backMoveValue_;
	currentState_ = State::MoveBack;

	// Y座標の固定値
	initPosY_ = player.GetTranslation().y;

	// 回転補間範囲内に入っていたら
	if (CheckInRange(attackLookAtCircleRange_, PlayerIState::GetDistanceToBossEnemy())) {

		// カメラアニメーション開始
		followCamera_->StartPlayerActionAnim(PlayerState::Attack_3rd);
	}
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

void PlayerAttack_3rdState::UpdateAlways([[maybe_unused]] Player& player) {

	// ダッシュエフェクトの更新
	// 親の回転を設定する
	catchDashEffect_->Update();
}

void PlayerAttack_3rdState::LerpWeapon(Player& player, PlayerWeaponType type) {

	if (player.GetUpdateMode() == ObjectUpdateMode::None &&
		currentState_ == State::None) {
		return;
	}

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

		// ダッシュエフェクトの発生
		catchDashEffect_->SetParentRotation("playerAttack3rdDashEffect",
			Quaternion::Normalize(player.GetRotation()), ParticleUpdateModuleID::Rotation);
		catchDashEffect_->SetParentRotation("playerAttack3rdDashEffect",
			Quaternion::Normalize(player.GetRotation()), ParticleUpdateModuleID::Primitive);
		catchDashEffect_->Emit(player_->GetTranslation() + (player.GetRotation() * dashEffectOffset_));

		// 残像表現エフェクト開始
		std::vector<GameObject3D*> objects = {
			&player,
			player.GetWeapon(PlayerWeaponType::Left),
			player.GetWeapon(PlayerWeaponType::Right)
		};
		afterImageEffect_->Start(objects);
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

		// Y軸回転オフセットをかける
		Vector3 rotated = RotateYOffset(player.GetTransform().GetForward(),
			weaponParams_[type].offsetRotationY).Normalize();

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

	// 残像表現エフェクト終了
	std::vector<GameObject3D*> objects = {
		&player,
		player.GetWeapon(PlayerWeaponType::Left),
		player.GetWeapon(PlayerWeaponType::Right)
	};
	afterImageEffect_->End(objects);

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
	player.ResetWeaponTransform(PlayerWeaponType::Left);
	player.ResetWeaponTransform(PlayerWeaponType::Right);

	// カメラアニメーションを終了させる
	followCamera_->EndPlayerActionAnim(false);
}

void PlayerAttack_3rdState::ImGui(const Player& player) {

	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);
	ImGui::DragFloat("bossEnemyDistance", &bossEnemyDistance_, 0.01f);
	ImGui::DragFloat("weaponPosY", &weaponPosY_, 0.01f);
	ImGui::DragFloat3("dashEffectOffset", &dashEffectOffset_.x, 0.1f);

	PlayerBaseAttackState::ImGui(player);

	ImGui::Separator();

	backMoveTimer_.ImGui("BackMoveTimer");
	ImGui::DragFloat("backMoveValue", &backMoveValue_, 0.1f);

	catchSwordTimer_.ImGui("CatchMoveTimer");

	weaponMoveTimer_.ImGui("WeaponMoveTimer");

	afterImageEffect_->ImGui();

	for (auto& [type, param] : weaponParams_) {

		ImGui::PushID(static_cast<uint32_t>(type));

		ImGui::SeparatorText(EnumAdapter<PlayerWeaponType>::ToString(type));

		ImGui::Text(std::format("isMoveStart: {}", param.isMoveStart).c_str());
		ImGui::DragFloat("moveValue", &param.moveValue, 0.1f);
		ImGui::DragFloat("rotateSpeed", &param.rotateSpeed, 0.01f);
		ImGui::DragFloat("offsetRotationY", &param.offsetRotationY, 0.01f);
		ImGui::DragFloat3("forward", &debugForward_[type].x);

		ImGui::PopID();

		// 飛ばされる予定の剣の位置
		Vector3 rotated = RotateYOffset(player.GetTransform().GetForward(),
			param.offsetRotationY);
		Vector3 target = bossEnemy_->GetTranslation() + rotated * bossEnemyDistance_;
		target.y = weaponPosY_;

		LineRenderer::GetInstance()->DrawSphere(6,
			8.0f, target, Color::Cyan());
	}
}

void PlayerAttack_3rdState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	exitTime_ = JsonAdapter::GetValue<float>(data, "exitTime_");
	bossEnemyDistance_ = JsonAdapter::GetValue<float>(data, "bossEnemyDistance_");
	weaponPosY_ = JsonAdapter::GetValue<float>(data, "weaponPosY_");

	dashEffectOffset_ = Vector3::FromJson(data.value("dashEffectOffset_", Json()));

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

	data["dashEffectOffset_"] = dashEffectOffset_.ToJson();

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