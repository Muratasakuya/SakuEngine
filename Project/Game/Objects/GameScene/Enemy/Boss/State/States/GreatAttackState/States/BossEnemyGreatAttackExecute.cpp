#include "BossEnemyGreatAttackExecute.h"

//============================================================================
//	BossEnemyGreatAttackExecute classMethods
//============================================================================

BossEnemyGreatAttackExecute::BossEnemyGreatAttackExecute() {

	// 初期化値
	canExit_ = false;
}

void BossEnemyGreatAttackExecute::Enter() {
}

void BossEnemyGreatAttackExecute::Update() {

	// 時間を更新
	nextTimer_.Update();

	// 時間経過後次の状態に進む
	if (nextTimer_.IsReached()) {

		canExit_ = true;
	}
}

void BossEnemyGreatAttackExecute::Exit() {

	// リセット
	nextTimer_.Reset();
	canExit_ = false;
}

void BossEnemyGreatAttackExecute::ImGui() {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::Separator();

	nextTimer_.ImGui("NextTimer", false);
}

void BossEnemyGreatAttackExecute::ApplyJson(const Json& data) {

	nextTimer_.FromJson(data.value("NextTimer", Json()));
}

void BossEnemyGreatAttackExecute::SaveJson(Json& data) {

	nextTimer_.ToJson(data["NextTimer"]);
}