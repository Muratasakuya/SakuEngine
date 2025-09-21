#include "RenderEngine.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Window/WinApp.h>
#include <Engine/Input/Input.h>
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/InstancedMeshSystem.h>
#include <Engine/Effect/Particle/Core/ParticleManager.h>
#include <Engine/Scene/SceneView.h>
#include <Engine/Config.h>

//============================================================================
//	RenderEngine classMethods
//============================================================================

void RenderEngine::InitDescriptor(ID3D12Device8* device) {

	// RTV初期化
	rtvDescriptor_ = std::make_unique<RTVDescriptor>();
	rtvDescriptor_->Init(device, DescriptorType(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));

	// DSV初期化
	dsvDescriptor_ = std::make_unique<DSVDescriptor>();
	dsvDescriptor_->Init(device, DescriptorType(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));

	// SRV初期化
	srvDescriptor_ = std::make_unique<SRVDescriptor>();
	srvDescriptor_->Init(device, DescriptorType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));

	// depthResourceの作成
	dsvDescriptor_->InitFrameBufferDSV();
}

void RenderEngine::InitRenderTextrue(ID3D12Device8* device) {

	RTVDescriptor* rtvDescriptor = rtvDescriptor_.get();
	SRVDescriptor* srvDescriptor = srvDescriptor_.get();

	renderTextures_[ViewType::Main] = std::make_unique<RenderTexture>();
	renderTextures_[ViewType::Main]->Create(
		// サイズ
		Config::kWindowWidth, Config::kWindowHeight,
		// 色
		Color(Config::kWindowClearColor[0], Config::kWindowClearColor[1],
			Config::kWindowClearColor[2], Config::kWindowClearColor[3]),
		// RTVフォーマット
		Config::kRenderTextureRTVFormat,
		device, rtvDescriptor, srvDescriptor);

#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	// gui用texture作成
	guiRenderTexture_ = std::make_unique<GuiRenderTexture>();
	guiRenderTexture_->Create(
		// サイズ
		Config::kWindowWidth, Config::kWindowHeight,
		// RTVフォーマット
		Config::kSwapChainRTVFormat,
		device, srvDescriptor);

	// デバッグ視点用
	renderTextures_[ViewType::Debug] = std::make_unique<RenderTexture>();
	renderTextures_[ViewType::Debug]->Create(
		// サイズ
		Config::kWindowWidth, Config::kWindowHeight,
		// 色
		Color(Config::kWindowClearColor[0], Config::kWindowClearColor[1],
			Config::kWindowClearColor[2], Config::kWindowClearColor[3]),
		// RTVフォーマット
		Config::kRenderTextureRTVFormat,
		device, rtvDescriptor, srvDescriptor);
#endif

	// SRV作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	uint32_t srvIndex = 0;
	srvDescriptor_->CreateSRV(srvIndex, dsvDescriptor_->GetResource(), srvDesc);
	dsvDescriptor_->SetFrameGPUHandle(srvDescriptor_->GetGPUHandle(srvIndex));
}

void RenderEngine::InitRenderer(ID3D12Device8* device, DxShaderCompiler* shaderCompiler) {

	// skinning処理用pipeline初期化
	skinningPipeline_ = std::make_unique<PipelineState>();
	skinningPipeline_->Create("Skinning.json", device, srvDescriptor_.get(), shaderCompiler);

	// renderer機能初期化
	meshRenderer_ = std::make_unique<MeshRenderer>();
	meshRenderer_->Init(device, shaderCompiler, srvDescriptor_.get());

	spriteRenderer_ = std::make_unique<SpriteRenderer>();
	spriteRenderer_->Init(device, srvDescriptor_.get(), shaderCompiler);
}

void RenderEngine::Init(WinApp* winApp, ID3D12Device8* device, DxShaderCompiler* shaderCompiler,
	DxCommand* dxCommand, IDXGIFactory7* factory) {

	objectManager_ = nullptr;
	objectManager_ = ObjectManager::GetInstance();

	dxCommand_ = nullptr;
	dxCommand_ = dxCommand;

	// scene初期化
	sceneBuffer_ = std::make_unique<SceneConstBuffer>();
	sceneBuffer_->Create(device);

	// descriptor初期化
	InitDescriptor(device);

	// 画面設定
	dxSwapChain_ = std::make_unique<DxSwapChain>();
	dxSwapChain_->Create(winApp, factory, dxCommand_->GetQueue(), rtvDescriptor_.get());

#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	imguiManager_ = std::make_unique<ImGuiManager>();
	imguiManager_->Init(winApp->GetHwnd(), dxSwapChain_->GetDesc().BufferCount, device, srvDescriptor_.get());

	pixelPicker_ = std::make_unique<GPUPixelPicker>();
	pixelPicker_->Init(device, shaderCompiler, srvDescriptor_.get());
#endif

	// renderTexture初期化
	InitRenderTextrue(device);

	// renderer初期化
	InitRenderer(device, shaderCompiler);
}

void RenderEngine::Finalize() {

#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	imguiManager_->Finalize();
	imguiManager_.reset();
#endif
}

void RenderEngine::BeginFrame() {

#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	imguiManager_->Begin();
#endif

	// srvDescriptorHeap設定
	dxCommand_->SetDescriptorHeaps({ srvDescriptor_->GetDescriptorHeap() });
}

void RenderEngine::UpdateGPUBuffer(SceneView* sceneView, bool enableMesh) {

	// commandList取得
	ID3D12GraphicsCommandList* commandList = dxCommand_->GetCommandList();

	// GPUResourceの更新
	sceneBuffer_->Update(sceneView);

	// skinningPipeline設定
	commandList->SetComputeRootSignature(skinningPipeline_->GetRootSignature());
	commandList->SetPipelineState(skinningPipeline_->GetComputePipeline());

	// TLASの更新処理
	// メッシュ非同期処理中は処理しない
	if (enableMesh) {

		// objectが持つbufferを更新
		objectManager_->UpdateBuffer();

		const auto& system = ObjectManager::GetInstance()->GetSystem<InstancedMeshSystem>();
		if (!system->IsBuilding()) {

			meshRenderer_->UpdateRayScene(dxCommand_);
		}
	}

#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	const RenderTarget renderTarget = renderTextures_[ViewType::Debug]->GetRenderTarget();
	Vector2 pixelInput = Vector2::AnyInit(0.0f);
	if (Input::GetInstance()->IsMouseOnView(InputViewArea::Scene)) {

		pixelInput = Input::GetInstance()->GetMousePosInView(InputViewArea::Scene).value();
	}
	pixelPicker_->Update(sceneView, Vector2(static_cast<float>(renderTarget.width),
		static_cast<float>(renderTarget.height)), pixelInput);
#endif
}

void RenderEngine::Rendering(ViewType type, bool enableMesh) {

	BeginRenderTarget(renderTextures_[type].get());

	// 描画処理
	Renderers(type, enableMesh);

	EndRenderTarget(renderTextures_[type].get());
}

void RenderEngine::BeginPostProcess() {

	// Write -> ComputeShader
	dxCommand_->TransitionBarriers({ dsvDescriptor_->GetResource() },
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void RenderEngine::EndPostProcess() {

	// ComputeShader -> Write
	dxCommand_->TransitionBarriers({ dsvDescriptor_->GetResource() },
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
}

void RenderEngine::Renderers(ViewType type, bool enableMesh) {

	bool isDebugEnable = static_cast<bool>(type);

	// 再設定
	// srvDescriptorHeap設定
	dxCommand_->SetDescriptorHeaps({ srvDescriptor_->GetDescriptorHeap() });

	// sprite描画、postPrecess適用
	// model描画前
	spriteRenderer_->ApplyPostProcessRendering(SpriteLayer::PreModel, sceneBuffer_.get(), dxCommand_);

	// line描画実行
	LineRenderer::GetInstance()->ExecuteLine(isDebugEnable, LineType::None);

	// 通常描画処理(シーン遷移中は処理できないようにする)
	if (enableMesh) {

		meshRenderer_->Rendering(isDebugEnable, sceneBuffer_.get(), dxCommand_);
	}

	// particle描画
	ParticleManager::GetInstance()->Rendering(isDebugEnable, sceneBuffer_.get(), dxCommand_);

	// line描画実行、depth無効
	LineRenderer::GetInstance()->ExecuteLine(isDebugEnable, LineType::DepthIgnore);

	// sprite描画、postPrecess適用
	// model描画後
	spriteRenderer_->ApplyPostProcessRendering(SpriteLayer::PostModel, sceneBuffer_.get(), dxCommand_);

	// ピッキング処理、左クリックしたときのみ更新
	if (Input::GetInstance()->IsMouseOnView(InputViewArea::Scene)) {
		if (type == ViewType::Debug &&
			Input::GetInstance()->TriggerMouse(MouseButton::Left)) {

			pixelPicker_->Execute(dxCommand_, meshRenderer_->GetTLASResource());
		}
	}
}

void RenderEngine::BeginRenderFrameBuffer() {

	const RenderTarget renderTarget = dxSwapChain_->GetRenderTarget();

	// RenderTarget -> Present
	dxCommand_->TransitionBarriers({ dxSwapChain_->GetCurrentResource() },
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	dxCommand_->SetRenderTargets(renderTarget, dsvDescriptor_->GetFrameCPUHandle());
	dxCommand_->SetViewportAndScissor(renderTarget.width, renderTarget.height);
}

void RenderEngine::EndRenderFrameBuffer() {

	// sprite描画、postPrecessを適用しない
	spriteRenderer_->IrrelevantRendering(sceneBuffer_.get(), dxCommand_);

#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	// gui描画用のtextureをframeBufferからコピー
	dxCommand_->CopyTexture(
		guiRenderTexture_->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		dxSwapChain_->GetCurrentResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// imgui描画
	imguiManager_->End();
	imguiManager_->Draw(dxCommand_->GetCommandList());

	// ComputeShader -> RenderTarget
	dxCommand_->TransitionBarriers({ renderTextures_[ViewType::Debug]->GetResource() },
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
#endif // _DEBUG

	// ComputeShader -> RenderTarget
	dxCommand_->TransitionBarriers({ renderTextures_[ViewType::Main]->GetResource() },
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	// Present -> RenderTarget
	dxCommand_->TransitionBarriers({ dxSwapChain_->GetCurrentResource() },
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

void RenderEngine::BeginRenderTarget(RenderTexture* renderTexture) {

	const RenderTarget renderTarget = renderTexture->GetRenderTarget();

	dxCommand_->SetRenderTargets(renderTarget, dsvDescriptor_->GetFrameCPUHandle());
	dxCommand_->ClearDepthStencilView(dsvDescriptor_->GetFrameCPUHandle());
	dxCommand_->SetViewportAndScissor(renderTarget.width, renderTarget.height);
}

void RenderEngine::EndRenderTarget(RenderTexture* renderTexture) {

	// RenderTarget -> ComputeShader
	dxCommand_->TransitionBarriers({ renderTexture->GetResource() },
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}
