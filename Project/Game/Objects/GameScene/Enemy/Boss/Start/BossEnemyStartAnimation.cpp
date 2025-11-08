#include "BossEnemyStartAnimation.h"

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	BossEnemyStartAnimation classMethods
//============================================================================

void BossEnemyStartAnimation::Init() {

	// 処理開始フラグ初期化
	isStarted_ = false;

	// json適応
	ApplyJson();
}

void BossEnemyStartAnimation::Update(BossEnemy& bossEnemy) {

	// 処理が開始されていなければ何もしない
	if (!isStarted_) {
		return;
	}

	// 座標補間更新
	Vector3 lerpTranslation = bossEnemy.GetTranslation();
	posAnimation_.LerpValue(lerpTranslation);

	// 座標を設定
	bossEnemy.SetTranslation(lerpTranslation);

	// 処理が終了したらその場に留める
	if (posAnimation_.IsFinished()) {

		isStarted_ = false;
		posAnimation_.Reset();
	}
}

void BossEnemyStartAnimation::Start(BossEnemy& bossEnemy) {

	// アニメーションを設定
	bossEnemy.SetNextAnimation("bossEnemy_start", false, 0.0f);

	// 補間開始
	isStarted_ = true;
	posAnimation_.Start();
}

void BossEnemyStartAnimation::ImGui(BossEnemy& bossEnemy) {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	if (ImGui::Button("Start")) {

		Start(bossEnemy);
	}
	ImGui::Separator();

	ImGui::Text(std::format("isStarted: {}", isStarted_).c_str());

	posAnimation_.ImGui("PosAnimation", false);
}

void BossEnemyStartAnimation::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Enemy/Boss/startAnimationParameter.json", data)) {
		return;
	}

	posAnimation_.FromJson(data["PosAnimation"]);
}

void BossEnemyStartAnimation::SaveJson() {

	Json data;

	posAnimation_.ToJson(data["PosAnimation"]);

	JsonAdapter::Save("Enemy/Boss/startAnimationParameter.json", data);
}