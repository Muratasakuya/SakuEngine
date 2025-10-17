#include "BossEnemyGreatAttackCharge.h"

//============================================================================
//	include
//============================================================================
#include <Game/Camera/Follow/FollowCamera.h>

//============================================================================
//	BossEnemyGreatAttackCharge classMethods
//============================================================================

BossEnemyGreatAttackCharge::BossEnemyGreatAttackCharge() {

	// 初期化値
	canExit_ = false;
}

void BossEnemyGreatAttackCharge::Enter() {
}

void BossEnemyGreatAttackCharge::Update() {

	// 時間を更新
	nextTimer_.Update();

	// 時間経過後次の状態に進む
	if (nextTimer_.IsReached()) {

		canExit_ = true;
	}
}

void BossEnemyGreatAttackCharge::Exit() {

	// リセット
	nextTimer_.Reset();
	canExit_ = false;
}

void BossEnemyGreatAttackCharge::ImGui() {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::Separator();

	nextTimer_.ImGui("NextTimer", false);
}

void BossEnemyGreatAttackCharge::ApplyJson(const Json& data) {

	nextTimer_.FromJson(data.value("NextTimer", Json()));
}

void BossEnemyGreatAttackCharge::SaveJson(Json& data) {

	nextTimer_.ToJson(data["NextTimer"]);
}