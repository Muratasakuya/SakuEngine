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
	isWaited_ = false;

	// json適応
	ApplyJson();
}

void BossEnemyStartAnimation::Update(BossEnemy& bossEnemy) {

	// 処理が開始されていなければ何もしない
	if (!isStarted_) {
		return;
	}

	if (!delayTimer_.IsReached()) {

		// 待機時間更新
		delayTimer_.Update();
		return;
	} else {
		if (!isWaited_) {
			// 時間経過後アニメーションさせる
			// アニメーションを設定
			bossEnemy.SetNextAnimation("bossEnemy_start", false, 0.0f);
			isWaited_ = true;
		}
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
		delayTimer_.Reset();
	}
}

void BossEnemyStartAnimation::Start(BossEnemy& bossEnemy) {

	// 補間開始
	isStarted_ = true;
	posAnimation_.Start();
	delayTimer_.Reset();
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

	delayTimer_.ImGui("DelayTimer", true);
	posAnimation_.ImGui("PosAnimation", false);
}

void BossEnemyStartAnimation::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Enemy/Boss/startAnimationParameter.json", data)) {
		return;
	}

	delayTimer_.FromJson(data.value("DelayTimer", Json()));
	posAnimation_.FromJson(data["PosAnimation"]);
}

void BossEnemyStartAnimation::SaveJson() {

	Json data;

	posAnimation_.ToJson(data["PosAnimation"]);

	delayTimer_.ToJson(data["DelayTimer"]);
	JsonAdapter::Save("Enemy/Boss/startAnimationParameter.json", data);
}