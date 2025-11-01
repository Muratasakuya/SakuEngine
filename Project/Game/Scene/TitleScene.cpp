#include "TitleScene.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Scene/SceneView.h>
#include <Engine/Scene/Manager/SceneManager.h>

// postEffect
#include <Game/PostEffect/RadialBlurUpdater.h>
#include <Game/PostEffect/GlitchUpdater.h>
#include <Game/PostEffect/CRTDisplayUpdater.h>

//============================================================================
//	TitleScene classMethods
//============================================================================

void TitleScene::Init() {

	//========================================================================
	//	postProcess
	//========================================================================

	PostProcessSystem* postProcess = PostProcessSystem::GetInstance();

	// 初期化時にのみ作成できる
	postProcess->Create({
		PostProcessType::RadialBlur,
		PostProcessType::Bloom,
		PostProcessType::CRTDisplay,
		PostProcessType::Glitch,
		PostProcessType::DepthBasedOutline,
		PostProcessType::Grayscale });
	// グリッチに使用するテクスチャを設定
	postProcess->InputProcessTexture("noise", PostProcessType::Glitch);

	// 更新クラスを登録
	postProcess->RegisterUpdater(std::make_unique<RadialBlurUpdater>());
	postProcess->RegisterUpdater(std::make_unique<GlitchUpdater>());
	postProcess->RegisterUpdater(std::make_unique<CRTDisplayUpdater>());

	// タイトルで使用するポストエフェクトを追加
	postProcess->AddProcess(PostProcessType::CRTDisplay);

	//========================================================================
	//	backObjects
	//========================================================================

	ObjectManager::GetInstance()->CreateSkybox("overcast_soil_puresky_4k");

	//========================================================================
	//	controller(objects)
	//========================================================================

	controller_ = std::make_unique<TitleController>();
	controller_->Init();

	//========================================================================
	//	scene
	//========================================================================

	// カメラの設定
	titleViewCamera_ = std::make_unique<TitleViewCamera>();
	titleViewCamera_->Init();

	sceneView_->SetGameCamera(titleViewCamera_.get());

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
	//	scene
	//========================================================================

	titleViewCamera_->Update();

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
	if (controller_->IsGameStart() && fadeTransition_ && sceneManager_->IsFinishedTransition()) {

		sceneManager_->SetNextScene(Scene::Game, std::move(fadeTransition_));
	}
}

void TitleScene::ImGui() {

	if (!fadeTransition_) {
		return;
	}
	fadeTransition_->ImGui();
}