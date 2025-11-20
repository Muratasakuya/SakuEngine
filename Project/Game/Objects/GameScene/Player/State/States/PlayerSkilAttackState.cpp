#include "PlayerSkilAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	PlayerSkilAttackState classMethods
//============================================================================

PlayerSkilAttackState::PlayerSkilAttackState(Player* player) {

	player_ = nullptr;
	player_ = player;

	// キーフレームオブジェクトの生成
	moveKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	moveKeyframeObject_->Init("playerSkilMoveKey");
	jumpKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	jumpKeyframeObject_->Init("playerSkilJumpKey");

	// 空の親トランスフォームの生成
	ObjectManager* objectManager = ObjectManager::GetInstance();
	uint32_t moveFrontID = objectManager->BuildEmptyobject("playerSkilMoveFrontTransform", "Player");
	moveFrontTag_ = objectManager->GetData<ObjectTag>(moveFrontID);
	// トランスフォームを追加
	moveFrontTransform_ = objectManager->GetObjectPoolManager()->AddData<Transform3D>(moveFrontID);
	moveFrontTransform_->Init();
	moveFrontTransform_->SetInstancingName(moveFrontTag_->name);
	// プレイヤーを親に設定
	moveFrontTransform_->parent = &player_->GetTransform();

	// 残像表現エフェクト作成
	afterImageEffect_ = std::make_unique<PlayerAfterImageEffect>();
	afterImageEffect_->Init("playerAttackSkilMove");
}

void PlayerSkilAttackState::Enter(Player& player) {

	// 最初のアニメーションに設定
	player.SetNextAnimation("player_skilAttack_1st", false, nextAnimDuration_);

	// 状態設定
	currentState_ = State::MoveAttack;
	canExit_ = false;
	exitTimer_ = 0.0f;

	// 敵が攻撃可能範囲にいるかチェックして目標を設定
	SetTargetByRange(*moveKeyframeObject_, "playerSkilMove");

	// キーフレーム補間開始
	moveKeyframeObject_->StartLerp();

	// 移動前座標を初期化
	preMovePos_ = player.GetTranslation();

	// 残像表現エフェクト開始
	std::vector<GameObject3D*> objects = {
		&player,
		player.GetWeapon(PlayerWeaponType::Left),
		player.GetWeapon(PlayerWeaponType::Right)
	};
	afterImageEffect_->Start(objects);

	// カメラアニメーション開始
	followCamera_->StartPlayerActionAnim("playerSkilMove");
}

void PlayerSkilAttackState::Update([[maybe_unused]] Player& player) {

	// 状態ごとの更新
	switch (currentState_) {
	case PlayerSkilAttackState::State::MoveAttack:

		// 移動攻撃更新
		UpdateMoveAttack(player);
		break;
	case PlayerSkilAttackState::State::JumpAttack:

		// ジャンプ攻撃更新
		UpdateJumpAttack(player);
		break;
	}
}

void PlayerSkilAttackState::UpdateMoveAttack(Player& player) {

	// トランスフォーム補間更新
	moveKeyframeObject_->SelfUpdate();

	// 補間された回転、座標をプレイヤーに適用
	Vector3 currentTranslation = moveKeyframeObject_->GetCurrentTransform().translation;
	player.SetTranslation(currentTranslation);
	// 回転は次の移動位置の方向を向くようにする
	// 方向
	Vector3 direction = Vector3(currentTranslation - preMovePos_).Normalize();
	Quaternion rotation = Quaternion::LookRotation(direction, rotationAxis_);
	player.SetRotation(Quaternion::Normalize(rotation));

	// 移動座標を更新する
	preMovePos_ = currentTranslation;

	// 補間処理終了後状態を終了
	if (!moveKeyframeObject_->IsUpdating()) {

		// 状態を進める
		currentState_ = State::JumpAttack;

		// 残像表現エフェクト終了
		std::vector<GameObject3D*> objects = {
			&player,
			player.GetWeapon(PlayerWeaponType::Left),
			player.GetWeapon(PlayerWeaponType::Right)
		};
		afterImageEffect_->End(objects);

		// ジャンプ攻撃アニメーションに設定
		player.SetNextAnimation("player_skilAttack_2nd", false, nextJumpAnimDuration_);

		// この時点の位置でまた範囲をチェックする
		// 敵が攻撃可能範囲にいるかチェックして目標を設定
		SetTargetByRange(*jumpKeyframeObject_, "cameraPlayerSkilJump");

		// 目標に対して回転を設定する
		Vector3 target = isInRange_ ? GetBossEnemyFixedYPos() : moveFrontTransform_->GetWorldPos();
		target.y = 0.0f;
		// target方向
		Vector3 lookDirection = Vector3::Normalize(target - GetPlayerFixedYPos());
		Quaternion targetRotation = Quaternion::LookRotation(lookDirection, Vector3(0.0f, 1.0f, 0.0f));
		// 最後の向きが反対なので逆回転にする
		player.SetRotation(Quaternion::Normalize(Quaternion::Conjugate(targetRotation)));

		// ジャンプキーフレーム補間開始
		jumpKeyframeObject_->StartLerp();
	}
}

void PlayerSkilAttackState::UpdateJumpAttack(Player& player) {

	// トランスフォーム補間更新
	jumpKeyframeObject_->SelfUpdate();

	// 補間処理終了後状態を終了
	if (!jumpKeyframeObject_->IsUpdating()) {

		// 経過時間更新
		exitTimer_ += GameTimer::GetDeltaTime();
		// 時間経過後に状態終了可能にする
		if (exitTime_ <= exitTimer_) {

			canExit_ = true;
		}
	} else {

		// 補間された回転、座標をプレイヤーに適用
		Vector3 currentTranslation = jumpKeyframeObject_->GetCurrentTransform().translation;
		player.SetTranslation(currentTranslation);
	}
}

void PlayerSkilAttackState::SetTargetByRange(KeyframeObject3D& keyObject, const std::string& cameraKeyName) {

	// 敵が攻撃可能範囲にいるかチェック
	isInRange_ = CheckInRange(attackLookAtCircleRange_, PlayerIState::GetDistanceToBossEnemy());
	if (isInRange_) {

		// 範囲内なので敵を親の位置として設定する
		keyObject.SetParent(bossEnemy_->GetTag().name, bossEnemy_->GetTransform());
		followCamera_->SetEditorParentTransform(cameraKeyName, bossEnemy_->GetTransform());
	} else {

		// 範囲外なので空の親トランスフォームを親の位置として設定する
		keyObject.SetParent(moveFrontTag_->name, *moveFrontTransform_);
		followCamera_->SetEditorParentTransform(cameraKeyName, *moveFrontTransform_);
	}
}

void PlayerSkilAttackState::UpdateAlways([[maybe_unused]] Player& player) {

	// キーフレームオブジェクトの更新
	moveFrontTransform_->UpdateMatrix();
	moveKeyframeObject_->UpdateKey();
	jumpKeyframeObject_->UpdateKey();
}

void PlayerSkilAttackState::Exit([[maybe_unused]] Player& player) {

	// リセット
	canExit_ = false;
	exitTimer_ = 0.0f;
	// 補間を確実に終了させる
	moveKeyframeObject_->Reset();
	jumpKeyframeObject_->Reset();
}

void PlayerSkilAttackState::ImGui([[maybe_unused]] const Player& player) {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());

	ImGui::Separator();

	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.001f);
	ImGui::DragFloat("nextJumpAnimDuration", &nextJumpAnimDuration_, 0.001f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);

	ImGui::DragFloat3("rotationAxis", &rotationAxis_.x, 0.01f);
	PlayerBaseAttackState::ImGui(player);

	ImGui::SeparatorText("MoveFront Effect");

	afterImageEffect_->ImGui();

	ImGui::SeparatorText("MoveFront Transform");

	moveFrontTransform_->ImGui(200.0f);

	LineRenderer::GetInstance()->DrawOBB(moveFrontTransform_->GetWorldPos(),
		moveFrontTransform_->scale, moveFrontTransform_->rotation, Color::Cyan());

	ImGui::SeparatorText("KeyframeObject3D");

	if (ImGui::CollapsingHeader("MoveKeyframeObject")) {

		moveKeyframeObject_->ImGui();
	}
	if (ImGui::CollapsingHeader("JumpKeyframeObject")) {

		jumpKeyframeObject_->ImGui();
	}
}

void PlayerSkilAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	nextJumpAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextJumpAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	exitTime_ = JsonAdapter::GetValue<float>(data, "exitTime_");

	PlayerBaseAttackState::ApplyJson(data);

	rotationAxis_ = Vector3::FromJson(data.value("rotationAxis_", Json()));

	moveFrontTransform_->FromJson(data.value("MoveFrontTransform", Json()));
	moveKeyframeObject_->FromJson(data.value("MoveKey", Json()));
	jumpKeyframeObject_->FromJson(data.value("JumpKey", Json()));
}

void PlayerSkilAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["nextJumpAnimDuration_"] = nextJumpAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["exitTime_"] = exitTime_;

	PlayerBaseAttackState::SaveJson(data);

	data["rotationAxis_"] = rotationAxis_.ToJson();

	moveFrontTransform_->ToJson(data["MoveFrontTransform"]);
	moveKeyframeObject_->ToJson(data["MoveKey"]);
	jumpKeyframeObject_->ToJson(data["JumpKey"]);
}