#include "BeginGameState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>

//============================================================================
//	BeginGameState classMethods
//============================================================================

void BeginGameState::Init([[maybe_unused]] SceneView* sceneView) {

	// json適応
	ApplyJson();
}

void BeginGameState::Update([[maybe_unused]] SceneManager* sceneManager) {

	const GameSceneState currentState = GameSceneState::BeginGame;

	//========================================================================
	//	object
	//========================================================================

	context_->player->Update();
	context_->boss->Update(currentState);

	//========================================================================
	//	scene
	//========================================================================

	context_->camera->Update(currentState);

	//========================================================================
	//	sceneEvent
	//========================================================================

	// ボスの登場演出が終了したらゲーム開始
	if (context_->camera->GetBeginGameCamera()->IsFinished()) {

		context_->fadeSprite->Start();
		// 遷移処理
		context_->fadeSprite->Update();
	}
	// 次の状態に遷移
	if (context_->fadeSprite->IsFinished()) {

		requestNext_ = true;
	}
}

void BeginGameState::NonActiveUpdate([[maybe_unused]] SceneManager* sceneManager) {
}

void BeginGameState::Exit() {

	// プレイヤーの座標をゲーム開始座標にする
	context_->player->SetTranslation(startPlayerPos_);
}

void BeginGameState::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	ImGui::DragFloat3("startPlayerPos", &startPlayerPos_.x, 0.1f);
	LineRenderer::GetInstance()->DrawSphere(8, 8.0f, startPlayerPos_,
		Color::Red());
}

void BeginGameState::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Scene/State/beginGameState.json", data)) {
		return;
	}

	startPlayerPos_ = Vector3::FromJson(data["startPlayerPos_"]);
}

void BeginGameState::SaveJson() {

	Json data;

	data["startPlayerPos_"] = startPlayerPos_.ToJson();

	JsonAdapter::Save("Scene/State/beginGameState.json", data);
}