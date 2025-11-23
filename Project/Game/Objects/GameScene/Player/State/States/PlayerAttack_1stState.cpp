#include "PlayerAttack_1stState.h"

//============================================================================
//	include
//============================================================================
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

	// 剣エフェクト作成
	slashEffect_ = std::make_unique<EffectGroup>();
	slashEffect_->Init("slashEffect1st", "PlayerEffect");
	slashEffect_->LoadJson("GameEffectGroup/Player/playerAttackSlashEffect_0.json");

	// 親の設定
	slashEffect_->SetParent("playerAttackSlash_0", player_->GetTransform());
}

void PlayerAttack_1stState::Enter(Player& player) {

	player.SetNextAnimation("player_attack_1st", false, nextAnimDuration_);
	canExit_ = false;

	// 敵が攻撃可能範囲にいるかチェック
	const Vector3 playerPos = player.GetTranslation();
	// 補間座標を設定
	if (!CheckInRange(attackPosLerpCircleRange_, PlayerIState::GetDistanceToBossEnemy())) {

		startPos_ = playerPos;
		targetPos_ = startPos_ + player.GetTransform().GetForward() * moveValue_;
	}

	// 回転補間範囲内にいるかどうか
	assisted_ = CheckInRange(attackLookAtCircleRange_, PlayerIState::GetDistanceToBossEnemy());
	if (assisted_) {

		// カメラの向きを補正させる
		followCamera_->StartLookToTarget(FollowCameraTargetType::Player,
			FollowCameraTargetType::BossEnemy, true, true, targetCameraRotateX_);

		startPos_ = playerPos;
		targetPos_ = startPos_ + player.GetTransform().GetForward() * moveValue_;

		// y座標を合わせる
		targetPos_.y = startPos_.y;
	}

	// 剣エフェクトの発生
	slashEffect_->Emit(player_->GetRotation() * slashEffectOffset_);
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

void PlayerAttack_1stState::UpdateAlways(Player& player) {

	// 剣エフェクトの更新、親の回転を設定する
	slashEffect_->SetParentRotation("playerAttackSlash_0",
		Quaternion::Normalize(player.GetRotation()), ParticleUpdateModuleID::Rotation);
	slashEffect_->Update();
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
	ImGui::DragFloat3("slashEffectOffset", &slashEffectOffset_.x, 0.1f);

	PlayerBaseAttackState::ImGui(player);

	moveTimer_.ImGui("MoveTimer");
	ImGui::DragFloat("moveValue", &moveValue_, 0.1f);
}

void PlayerAttack_1stState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	exitTime_ = JsonAdapter::GetValue<float>(data, "exitTime_");

	slashEffectOffset_ = Vector3::FromJson(data.value("slashEffectOffset_", Json()));

	PlayerBaseAttackState::ApplyJson(data);

	moveTimer_.FromJson(data.value("MoveTimer", Json()));
	moveValue_ = data.value("moveValue_", 1.0f);
	targetCameraRotateX_ = data.value("targetCameraRotateX_", 0.0f);
}

void PlayerAttack_1stState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["exitTime_"] = exitTime_;

	data["slashEffectOffset_"] = slashEffectOffset_.ToJson();

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