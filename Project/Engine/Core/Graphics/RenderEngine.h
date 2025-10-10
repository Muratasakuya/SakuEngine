#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxObject/DxSwapChain.h>
#include <Engine/Core/Graphics/PostProcess/Texture/RenderTexture.h>
#include <Engine/Core/Graphics/PostProcess/Texture/MultiRenderTexture.h>
#include <Engine/Core/Graphics/Descriptors/RTVDescriptor.h>
#include <Engine/Core/Graphics/Descriptors/DSVDescriptor.h>
#include <Engine/Core/Graphics/Descriptors/SRVDescriptor.h>
#include <Engine/External/ImGuiManager.h>

// scene
#include <Engine/Core/Graphics/GPUObject/SceneConstBuffer.h>
#include <Engine/Core/Graphics/GPUObject/GPUPixelPicker.h>

// renderer
#include <Engine/Core/Graphics/Renderer/MeshRenderer.h>
#include <Engine/Core/Graphics/Renderer/SpriteRenderer.h>

// front
class SceneView;
class WinApp;
class ObjectManager;
class DxCommand;

//============================================================================
//	RenderEngine class
//============================================================================
class RenderEngine {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	//--------- enum class ---------------------------------------------------

	// 描画シーン
	enum class ViewType {

		Main,  //メイン
		Debug, // デバッグ視点用
	};

	// RTVの描画
	enum class SVTarget {

		Color,           // 色
		PostProcessMask, // ポストプロセスのビットマスク
	};
public:
	//========================================================================
	//	public Methods
	//========================================================================

	RenderEngine() = default;
	~RenderEngine() = default;

	void Init(WinApp* winApp, ID3D12Device8* device, DxShaderCompiler* shaderCompiler,
		DxCommand* dxCommand, IDXGIFactory7* factory);

	void Finalize();

	void BeginFrame();

	// GPUの更新処理
	void UpdateGPUBuffer(SceneView* sceneView, bool enableMesh);

	// viewごとの描画
	void Rendering(ViewType type, bool enableMesh);

	// postProcess処理
	void BeginPostProcess();
	void EndPostProcess();

	// swapChain描画
	void BeginRenderFrameBuffer();
	void EndRenderFrameBuffer();

	//--------- accessor -----------------------------------------------------

	SRVDescriptor* GetSRVDescriptor() const { return srvDescriptor_.get(); }
	RTVDescriptor* GetRTVDescriptor() const { return rtvDescriptor_.get(); }
	DSVDescriptor* GetDSVDescriptor() const { return dsvDescriptor_.get(); }

	DxSwapChain* GetDxSwapChain() const { return dxSwapChain_.get(); }

	RenderTexture* GetRenderTexture(ViewType type, SVTarget target) const;

	const D3D12_GPU_DESCRIPTOR_HANDLE& GetDepthGPUHandle() const { return dsvDescriptor_->GetFrameGPUHandle(); }
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetRenderTextureGPUHandle() const { return guiRenderTexture_->GetGPUHandle(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ObjectManager* objectManager_;
	DxCommand* dxCommand_;

	std::unique_ptr<DxSwapChain> dxSwapChain_;

	// renderTexture
	std::unordered_map<ViewType, std::unique_ptr<MultiRenderTexture>> multiRenderTextures_;
	// frameBufferからコピーする用
	std::unique_ptr<GuiRenderTexture> guiRenderTexture_;

	// descriptor
	std::unique_ptr<RTVDescriptor> rtvDescriptor_;
	std::unique_ptr<DSVDescriptor> dsvDescriptor_;
	std::unique_ptr<SRVDescriptor> srvDescriptor_;

	// pipeline
	std::unique_ptr<PipelineState> skinningPipeline_;

	// scene
	std::unique_ptr<SceneConstBuffer> sceneBuffer_;

	// renderer
	std::unique_ptr<MeshRenderer> meshRenderer_;
	std::unique_ptr<SpriteRenderer> spriteRenderer_;

	// imgui
	std::unique_ptr<ImGuiManager> imguiManager_;
	std::unique_ptr<GPUPixelPicker> pixelPicker_;

	//--------- functions ----------------------------------------------------

	// init
	void InitDescriptor(ID3D12Device8* device);
	void InitRenderTextrue(ID3D12Device8* device);
	void InitRenderer(ID3D12Device8* device, DxShaderCompiler* shaderCompiler);

	// render
	void Renderers(ViewType type, bool enableMesh);

	// command
	void BeginRenderTarget(MultiRenderTexture* multiRenderTexture);
	void EndRenderTarget(MultiRenderTexture* multiRenderTexture);

	// helper
	MultiRenderTexture* CreateViewMRT(ViewType type, ID3D12Device8* device);
	void AddDefaultAttachments(MultiRenderTexture* multiRenderTexture);
	void CreateGuiRenderTexture(ID3D12Device8* device);
	void CreateDepthSRV();
};