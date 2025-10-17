#include "BossEnemyGreatAttackBlowPlayer.h"

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Camera/Follow/FollowCamera.h>

//============================================================================
//	BossEnemyGreatAttackBlowPlayer classMethods
//============================================================================

BossEnemyGreatAttackBlowPlayer::BossEnemyGreatAttackBlowPlayer() {

	// アニメーションを初期化
	animTranslation_ = std::make_unique<SimpleAnimation<Vector3>>();

	// 初期化値
	canExit_ = false;
}

void BossEnemyGreatAttackBlowPlayer::Enter() {

	// 画面の中心にテレポートさせる
	// 座標を設定
	animTranslation_->SetStart(bossEnemy_->GetTranslation());
	// 終了座標
	animTranslation_->SetEnd(Vector3::AnyInit(0.0f));

	// アニメーション開始
	animTranslation_->Start();
}

void BossEnemyGreatAttackBlowPlayer::Update() {

	// 座標を補間
	Vector3 translation = bossEnemy_->GetTranslation();
	animTranslation_->LerpValue(translation);

	// 座標を設定
	bossEnemy_->SetTranslation(translation);

	// 補間終了後次の状態進ませる
	if (animTranslation_->IsFinished()) {

		canExit_ = true;
	}
}

void BossEnemyGreatAttackBlowPlayer::Exit() {

	// リセット
	animTranslation_->Reset();
	canExit_ = false;
}

void BossEnemyGreatAttackBlowPlayer::ImGui() {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::Separator();

	animTranslation_->ImGui("Translation", false);
}

void BossEnemyGreatAttackBlowPlayer::ApplyJson(const Json& data) {

	animTranslation_->FromJson(data.value("AnimTranslation", Json()));
}

void BossEnemyGreatAttackBlowPlayer::SaveJson(Json& data) {

	animTranslation_->ToJson(data["AnimTranslation"]);
}