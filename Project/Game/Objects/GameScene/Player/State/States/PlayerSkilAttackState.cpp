#include "PlayerSkilAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	PlayerSkilAttackState classMethods
//============================================================================

PlayerSkilAttackState::PlayerSkilAttackState([[maybe_unused]] Player* player) {

	player_ = nullptr;
	player_ = player;

	// キーフレームオブジェクトの生成
	moveKeyframeObject_ = std::make_unique<KeyframeObject3D>();
	// キーの追加
	moveKeyframeObject_->AddKeyValue(AnyMold::Float, "TestFloat");
	moveKeyframeObject_->AddKeyValue(AnyMold::Color, "TestColor");
	moveKeyframeObject_->Init("playerSkilMoveKey");
}

void PlayerSkilAttackState::Enter([[maybe_unused]] Player& player) {

}

void PlayerSkilAttackState::Update([[maybe_unused]] Player& player) {

	canExit_ = true;
}

void PlayerSkilAttackState::Exit([[maybe_unused]] Player& player) {

	// リセット
	canExit_ = false;
}

void PlayerSkilAttackState::ImGui([[maybe_unused]] const Player& player) {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());

	ImGui::Separator();

	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.001f);
	ImGui::DragFloat("exitTime", &exitTime_, 0.01f);

	ImGui::SeparatorText("KeyframeObject3D");

	moveKeyframeObject_->ImGui();
}

void PlayerSkilAttackState::ApplyJson(const Json& data) {

	nextAnimDuration_ = JsonAdapter::GetValue<float>(data, "nextAnimDuration_");
	rotationLerpRate_ = JsonAdapter::GetValue<float>(data, "rotationLerpRate_");
	exitTime_ = JsonAdapter::GetValue<float>(data, "exitTime_");

	PlayerBaseAttackState::ApplyJson(data);

	moveKeyframeObject_->FromJson(data.value("MoveKey", Json()));
}

void PlayerSkilAttackState::SaveJson(Json& data) {

	data["nextAnimDuration_"] = nextAnimDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["exitTime_"] = exitTime_;

	PlayerBaseAttackState::SaveJson(data);

	moveKeyframeObject_->ToJson(data["MoveKey"]);
}

bool PlayerSkilAttackState::GetCanExit() const {

	// 経過時間が過ぎたら
	bool canExit = exitTimer_ > exitTime_;
	return canExit;
}