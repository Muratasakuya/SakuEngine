#include "BossEnemyGreatAttackFinish.h"

//============================================================================
//	BossEnemyGreatAttackFinish classMethods
//============================================================================

BossEnemyGreatAttackFinish::BossEnemyGreatAttackFinish() {

	// 初期化値
	canExit_ = false;
}

void BossEnemyGreatAttackFinish::Enter() {
}

void BossEnemyGreatAttackFinish::Update() {

	// 時間を更新
	nextTimer_.Update();

	// 時間経過後状態を終了し元のコンボシークエンスに戻す
	if (nextTimer_.IsReached()) {

		canExit_ = true;
	}
}

void BossEnemyGreatAttackFinish::Exit() {

	// リセット
	nextTimer_.Reset();
	canExit_ = false;
}

void BossEnemyGreatAttackFinish::ImGui() {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::Separator();

	nextTimer_.ImGui("NextTimer", false);
}

void BossEnemyGreatAttackFinish::ApplyJson(const Json& data) {

	nextTimer_.FromJson(data.value("NextTimer", Json()));
}

void BossEnemyGreatAttackFinish::SaveJson(Json& data) {

	nextTimer_.ToJson(data["NextTimer"]);
}