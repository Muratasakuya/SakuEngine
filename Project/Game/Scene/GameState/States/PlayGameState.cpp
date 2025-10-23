#include "PlayGameState.h"

//============================================================================
//	PlayGameState classMethods
//============================================================================

void PlayGameState::Init([[maybe_unused]] SceneView* sceneView) {

	effectGroup_ = std::make_unique<GameEffectGroup>();
	effectGroup_->Init("testEffect", "Effect");
}

void PlayGameState::Update([[maybe_unused]] SceneManager* sceneManager) {

	const GameSceneState currentState = GameSceneState::PlayGame;

	//========================================================================
	//	debug
	//========================================================================

	effectGroup_->Update();

	//========================================================================
	//	object
	//========================================================================

	context_->boss->Update(currentState);
	context_->player->Update();

	// 時間経過を計測
	context_->result->Measurement();

	//========================================================================
	//	sprite
	//========================================================================

	// 遷移後処理
	context_->fadeSprite->Update();

	//========================================================================
	//	scene
	//========================================================================

	context_->camera->Update(currentState);
	context_->level->Update();

	// 移動範囲を制限する
	context_->fieldBoundary->ControlTargetMove();

	//========================================================================
	//	sceneEvent
	//========================================================================

	// プレイヤーか敵が死んだらクリアシーンに遷移させる
	if (context_->player->IsDead() ||
		context_->boss->IsDead()) {

		requestNext_ = true;

		// HUDの表示を消す
		context_->player->GetHUD()->SetDisable();
		context_->boss->GetHUD()->SetDisable();
	}
}

void PlayGameState::NonActiveUpdate([[maybe_unused]] SceneManager* sceneManager) {
}

void PlayGameState::ImGui() {

	context_->fieldBoundary->ImGui();
}