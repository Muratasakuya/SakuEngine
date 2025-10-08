#include "EffectScene.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Engine/Scene/SceneView.h>

//============================================================================
//	EffectScene classMethods
//============================================================================

void EffectScene::Init() {

	//========================================================================
	//	postProcess
	//========================================================================

	// 初期化時にのみ作成できる
	PostProcessSystem::GetInstance()->Create({ PostProcessType::RadialBlur,PostProcessType::Bloom });

	//========================================================================
	//	scene
	//========================================================================

	// カメラの設定
	camera3D_ = std::make_unique<BaseCamera>();
	sceneView_->SetGameCamera(camera3D_.get());

	// ライトの設定
	light_ = std::make_unique<PunctualLight>();
	light_->Init();
	sceneView_->SetLight(light_.get());
}

void EffectScene::Update() {

	camera3D_->UpdateView();

	LineRenderer::GetInstance()->DrawGrid(32, 32.0f, Color::White());
}