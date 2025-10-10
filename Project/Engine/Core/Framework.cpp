#include "Framework.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/SpdLogger.h>
#include <Engine/Input/Input.h>
#include <Engine/Asset/AssetEditor.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Collision/CollisionManager.h>
#include <Engine/Effect/Particle/Core/ParticleManager.h>
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Config.h>
#include <Engine/Editor/Camera/3D/Camera3DEditor.h>

//============================================================================
//	Framework classMethods
//============================================================================

void Framework::Run() {

	while (true) {

		Update();

		Draw();

		EndRequest();

		// fullScreen切り替え
		if (Input::GetInstance()->TriggerKey(DIK_F11)) {

			fullscreenEnable_ = !fullscreenEnable_;
			winApp_->SetFullscreen(fullscreenEnable_);
		}

		if (winApp_->ProcessMessage() ||
			sceneManager_->IsFinishGame()) {
			break;
		}
	}

	Finalize();
}

Framework::Framework() {

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	//========================================================================
	//	engineConfig
	//========================================================================

	SpdLogger::Init();
	SpdLogger::InitAsset();
	SpdLogger::Log("[StartLogginig]\n");

	LOG_INFO("\nconfigs\nwindowTitle: {}\nwindowSize: {}×{}\nmaxInstanceCount: {}\n\n",
		Config::kWindowTitleName, Config::kWindowWidth, Config::kWindowHeight, Config::kMaxInstanceNum);

	//========================================================================
	//	init
	//========================================================================

	fullscreenEnable_ = Config::kFullscreenEnable;

	// window作成
	winApp_ = std::make_unique<WinApp>();
	winApp_->Create();

	//------------------------------------------------------------------------
	// scene初期化

	sceneView_ = std::make_unique<SceneView>();
	sceneView_->Init();

	//------------------------------------------------------------------------
	// directX機能初期化

	graphicsPlatform_ = std::make_unique<GraphicsPlatform>();
	graphicsPlatform_->Init();

	ID3D12Device8* device = graphicsPlatform_->GetDevice();
	DxCommand* dxCommand = graphicsPlatform_->GetDxCommand();
	DxShaderCompiler* shaderCompiler = graphicsPlatform_->GetDxShaderCompiler();

	renderEngine_ = std::make_unique<RenderEngine>();
	renderEngine_->Init(winApp_.get(), device, shaderCompiler, dxCommand, graphicsPlatform_->GetDxgiFactory());

	SRVDescriptor* srvDescriptor = renderEngine_->GetSRVDescriptor();

	// asset機能初期化
	asset_ = std::make_unique<Asset>();
	asset_->Init(device, dxCommand, srvDescriptor);

	PostProcessSystem* postProcessSystem = PostProcessSystem::GetInstance();
	postProcessSystem->Init(device, shaderCompiler,
		srvDescriptor, asset_.get(), sceneView_.get());
	postProcessSystem->SetDepthFrameBufferGPUHandle(renderEngine_->GetDepthGPUHandle());

	// fullScreen設定
	if (fullscreenEnable_) {

		winApp_->SetFullscreen(fullscreenEnable_);
	}

	// srvDescriptorHeap設定、最初のみ設定
	dxCommand->SetDescriptorHeaps({ srvDescriptor->GetDescriptorHeap() });

	//------------------------------------------------------------------------
	// object機能初期化

	ObjectManager::GetInstance()->Init(device, asset_.get(), dxCommand);

	//------------------------------------------------------------------------
	// particle管理クラス初期化

	ParticleManager::GetInstance()->Init(asset_.get(),
		device, srvDescriptor, shaderCompiler);

	//------------------------------------------------------------------------
	// scene管理クラス初期化

	sceneManager_ = std::make_unique<SceneManager>(Scene::Title, asset_.get(), sceneView_.get());

	//------------------------------------------------------------------------
	// module初期化

	Input::GetInstance()->Init(winApp_.get());
	LineRenderer::GetInstance()->Init(device, dxCommand->GetCommandList(),
		srvDescriptor, shaderCompiler, sceneView_.get());
	AssetEditor::GetInstance()->Init(asset_.get());
	Camera3DEditor::GetInstance()->Init(sceneView_.get());

	//------------------------------------------------------------------------
	// imgui機能初期化

#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	imguiEditor_ = std::make_unique<ImGuiEditor>();
	imguiEditor_->Init(renderEngine_->GetRenderTextureGPUHandle(),
		postProcessSystem->GetCopySRVGPUHandle());
	imguiEditor_->LoadIconTextures(asset_.get());

	// console表示用
	imguiEditor_->SetConsoleViewDescriptor(DescriptorHeapType::SRV, srvDescriptor);
	imguiEditor_->SetConsoleViewDescriptor(DescriptorHeapType::RTV, renderEngine_->GetRTVDescriptor());
	imguiEditor_->SetConsoleViewDescriptor(DescriptorHeapType::DSV, renderEngine_->GetDSVDescriptor());
#endif
}

void Framework::Update() {

	//========================================================================
	//	update
	//========================================================================

	GameTimer::BeginFrameCount();
	GameTimer::BeginUpdateCount();

	// 描画前処理
	renderEngine_->BeginFrame();

	// 非同期読み込みの更新
	asset_->PumpAsyncLoads();
	// 読み込みがすべて終了したら次のシーンを初期化
	if (sceneManager_->ConsumeNeedInitNextScene()) {

		PostProcessSystem::GetInstance()->ClearProcess();
		sceneManager_->InitNextScene();
	}

	// シーン開始
	sceneManager_->BeginFrame();

	// imgui表示更新
	bool playGame = true;
#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	imguiEditor_->Display(sceneView_.get());
	playGame = imguiEditor_->IsPlayGame();
#endif

	// falseなら処理しない
	if (playGame) {

		// scene更新
		UpdateScene();
	}
	// シーン終了
	sceneManager_->EndFrame();
	GameTimer::EndUpdateCount();
}
void Framework::UpdateScene() {

	GameTimer::Update();
	Input::GetInstance()->Update();

	// scene更新
	sceneManager_->Update();
	sceneView_->Update();

	// collision更新
	CollisionManager::GetInstance()->Update();
	// data更新
	ObjectManager::GetInstance()->UpdateData();
	// particle更新
	ParticleManager::GetInstance()->Update(graphicsPlatform_->GetDxCommand());
	// postProcess更新
	PostProcessSystem::GetInstance()->Update();
}

void Framework::Draw() {

	DxCommand* dxCommand = graphicsPlatform_->GetDxCommand();

	//========================================================================
	//	draw: endFrame
	//========================================================================

	GameTimer::BeginDrawCount();

	// GPUの更新処理
	renderEngine_->UpdateGPUBuffer(sceneView_.get(), sceneManager_->IsMeshRenderingAllowed());

	//========================================================================
	//	draw: render
	//========================================================================

	// 描画処理
	RenderPath(dxCommand);

	//========================================================================
	//	draw: execute
	//========================================================================

	renderEngine_->EndRenderFrameBuffer();

	// csへの書き込み状態へ遷移
	PostProcessSystem::GetInstance()->ToWrite(dxCommand);

	// command実行
	dxCommand->ExecuteCommands(renderEngine_->GetDxSwapChain()->Get());

	GameTimer::EndDrawCount();
	GameTimer::EndFrameCount();
}

void Framework::RenderPath(DxCommand* dxCommand) {

	PostProcessSystem* postProcessSystem = PostProcessSystem::GetInstance();
	bool meshEnable = sceneManager_->IsMeshRenderingAllowed();

	//========================================================================
	//	draw: renderTexture
	//========================================================================

	renderEngine_->Rendering(RenderEngine::ViewType::Main, meshEnable);

	renderEngine_->BeginPostProcess();

	// postProcess処理実行
	postProcessSystem->Execute(dxCommand,
		renderEngine_->GetRenderTexture(RenderEngine::ViewType::Main,
			RenderEngine::SVTarget::Color)->GetSRVGPUHandle(),            // 0.色
		renderEngine_->GetRenderTexture(RenderEngine::ViewType::Main,
			RenderEngine::SVTarget::PostProcessMask)->GetSRVGPUHandle()); // 1.マスク

	renderEngine_->EndPostProcess();

	//========================================================================
	//	draw: debugViewRenderTexture
	//========================================================================
#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	renderEngine_->Rendering(RenderEngine::ViewType::Debug, meshEnable);

	postProcessSystem->ExecuteDebugScene(dxCommand,
		renderEngine_->GetRenderTexture(RenderEngine::ViewType::Debug,
			RenderEngine::SVTarget::Color)->GetSRVGPUHandle(),            // 0.色
		renderEngine_->GetRenderTexture(RenderEngine::ViewType::Debug,
			RenderEngine::SVTarget::PostProcessMask)->GetSRVGPUHandle()); // 1.マスク
#endif
	//========================================================================
	//	draw: frameBuffer
	//========================================================================

	renderEngine_->BeginRenderFrameBuffer();

	// frameBufferへ結果を描画
	postProcessSystem->RenderFrameBuffer(dxCommand);
}

void Framework::EndRequest() {

	// scene遷移依頼
	sceneManager_->SwitchScene();
	// lineReset
	LineRenderer::GetInstance()->ResetLine();
}

void Framework::Finalize() {

	// 全てのログ出力
	asset_->ReportUsage(true);

	graphicsPlatform_->Finalize(winApp_->GetHwnd());
	renderEngine_->Finalize();
	Input::GetInstance()->Finalize();
	LineRenderer::GetInstance()->Finalize();

	sceneManager_.reset();

	ObjectManager::GetInstance()->Finalize();
	ParticleManager::GetInstance()->Finalize();
	PostProcessSystem::GetInstance()->Finalize();

	winApp_.reset();
	asset_.reset();
	graphicsPlatform_.reset();
	renderEngine_.reset();
	sceneView_.reset();
	imguiEditor_.reset();

	// ComFinalize
	CoUninitialize();
}