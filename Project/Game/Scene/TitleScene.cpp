#include "TitleScene.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Engine/Scene/SceneView.h>
#include <Engine/Scene/Manager/SceneManager.h>

//============================================================================
//	TitleScene classMethods
//============================================================================

void TitleScene::Init() {

	//========================================================================
	//	postProcess
	//========================================================================

	// 初期化時にのみ作成できる
	PostProcessSystem::GetInstance()->Create({
		PostProcessType::RadialBlur,
		PostProcessType::Bloom,
		PostProcessType::CRTDisplay,
		PostProcessType::Glitch,
		PostProcessType::DepthBasedOutline,
		PostProcessType::Grayscale });

	PostProcessSystem::GetInstance()->InputProcessTexture("noise", PostProcessType::Glitch);

	//========================================================================
	//	controller(objects)
	//========================================================================

	controller_ = std::make_unique<TitleController>();
	controller_->Init();

	//========================================================================
	//	scene
	//========================================================================

	// カメラの設定
	camera3D_ = std::make_unique<BaseCamera>();
	camera3D_->UpdateView();

	sceneView_->SetGameCamera(camera3D_.get());

	// ライトの設定
	light_ = std::make_unique<PunctualLight>();
	light_->Init();
	sceneView_->SetLight(light_.get());

	// 遷移の設定
	fadeTransition_ = std::make_unique<FadeTransition>();
	fadeTransition_->Init();
}

void TitleScene::Update() {

	//========================================================================
	//	controller
	//========================================================================

	controller_->Update();

	//========================================================================
	//	config
	//========================================================================

	// ゲーム終了フラグの更新
	if (controller_->IsSelectFinish()) {

		// 終了させる
		isFinishGame_ = true;
	}

	//========================================================================
	//	sceneEvent
	//========================================================================

	// ゲーム開始フラグが立てば開始
	if (controller_->IsGameStart() &&
		fadeTransition_ && sceneManager_->IsFinishedTransition()) {

		sceneManager_->SetNextScene(Scene::Game, std::move(fadeTransition_));
	}
}

void TitleScene::ImGui() {

	if (!fadeTransition_) {
		return;
	}

	fadeTransition_->ImGui();
}