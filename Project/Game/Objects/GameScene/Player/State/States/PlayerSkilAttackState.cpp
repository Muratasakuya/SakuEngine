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
}

void PlayerSkilAttackState::Enter([[maybe_unused]] Player& player) {

	canExit_ = false;
	exitTimer_ = 0.0f;

	// 敵が攻撃可能範囲にいるかチェック
	// 補間座標を設定
	if (CheckInRange(attackPosLerpCircleRange_, PlayerIState::GetDistanceToBossEnemy())) {

		// 範囲内なので敵を親の位置として設定する
		moveKeyframeObject_->SetParent(bossEnemy_->GetTag().name, bossEnemy_->GetTransform());
	} else {

		// 範囲外なので空の親トランスフォームを親の位置として設定する
		moveKeyframeObject_->SetParent(moveFrontTag_->name, *moveFrontTransform_);
	}

	// キーフレーム補間開始
	moveKeyframeObject_->StartLerp();

	// 移動前座標を初期化
	preMovePos_ = player.GetTranslation();
}

void PlayerSkilAttackState::Update([[maybe_unused]] Player& player) {

	// トランスフォーム補間更新
	moveKeyframeObject_->SelfUpdate();

	// 補間処理終了後状態を終了
	if (!moveKeyframeObject_->IsUpdating()) {

		// 経過時間を加算、時間経過で勝手に遷移可能になる
		exitTimer_ += GameTimer::GetDeltaTime();
	} else {

		// 補間された回転、座標をプレイヤーに適用
		Vector3 currentTranslation = moveKeyframeObject_->GetCurrentTransform().translation;
		player.SetTranslation(currentTranslation);
		// 回転は次の移動位置の方向を向くようにする
		// 方向
		Vector3 direction = Vector3(currentTranslation - preMovePos_).Normalize();
		Quaternion rotation = Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f));
		player.SetRotation(Quaternion::Normalize(rotation));

		// 移動座標を更新する
		preMovePos_ = currentTranslation;
	}
}

void PlayerSkilAttackState::UpdateAlways([[maybe_unused]] Player& player) {

	// キーフレームオブジェクトの更新
	moveFrontTransform_->UpdateMatrix();
	moveKeyframeObject_->UpdateKey();
}

void PlayerSkilAttackState::Exit([[maybe_unused]] Player& player) {

	// リセット
	canExit_ = false;
	exitTimer_ = 0.0f;
	// 補間を確実に終了させる
	moveKeyframeObject_->Reset();
}

void PlayerSkilAttackState::ImGui([[maybe_unused]] const Player& player) {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());

	ImGui::Separator();

	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);

	PlayerBaseAttackState::ImGui(player);

	ImGui::SeparatorText("MoveFront Transform");

	moveFrontTransform_->ImGui(200.0f);

	LineRenderer::GetInstance()->DrawOBB(moveFrontTransform_->GetWorldPos(),
		moveFrontTransform_->scale, moveFrontTransform_->rotation, Color::Cyan());

	ImGui::SeparatorText("KeyframeObject3D");

	moveKeyframeObject_->ImGui();
}

void PlayerSkilAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	exitTime_ = JsonAdapter::GetValue<float>(data, "exitTime_");

	PlayerBaseAttackState::ApplyJson(data);

	moveFrontTransform_->FromJson(data.value("MoveFrontTransform", Json()));
	moveKeyframeObject_->FromJson(data.value("MoveKey", Json()));
}

void PlayerSkilAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["exitTime_"] = exitTime_;

	PlayerBaseAttackState::SaveJson(data);

	moveFrontTransform_->ToJson(data["MoveFrontTransform"]);
	moveKeyframeObject_->ToJson(data["MoveKey"]);
}

bool PlayerSkilAttackState::GetCanExit() const {

	// 経過時間が過ぎたら
	bool canExit = exitTimer_ > exitTime_;
	return canExit;
}