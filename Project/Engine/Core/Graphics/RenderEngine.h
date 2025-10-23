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
//	描画全体を統括し、レンダーターゲット/MRT/ポストプロセス/GUI描画を管理する。
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

	// スワップチェイン/各Descriptor/レンダラ/GUI等を初期化する
	void Init(WinApp* winApp, ID3D12Device8* device, DxShaderCompiler* shaderCompiler,
		DxCommand* dxCommand, IDXGIFactory7* factory);

	// GUIなどの終了処理を行う
	void Finalize();

	// フレーム先頭でのセットアップ(DescriptorHeapなど)を行う
	void BeginFrame();

	// シーン定数バッファやTLASなどGPU側の更新を行う
	void UpdateGPUBuffer(SceneView* sceneView, bool enableMesh);

	// 指定ビュー種類で描画パスを実行する
	void Rendering(ViewType type, bool enableMesh);

	// ポストプロセス前後のリソース状態遷移を行う
	void BeginPostProcess();
	void EndPostProcess();

	// フレームバッファ(スワップチェイン)への描画開始/終了を行う
	void BeginRenderFrameBuffer();
	void EndRenderFrameBuffer();

	//--------- accessor -----------------------------------------------------

	// 各DescriptorとSwapChain、レンダーテクスチャを取得する
	SRVDescriptor* GetSRVDescriptor() const { return srvDescriptor_.get(); }
	RTVDescriptor* GetRTVDescriptor() const { return rtvDescriptor_.get(); }
	DSVDescriptor* GetDSVDescriptor() const { return dsvDescriptor_.get(); }
	DxSwapChain* GetDxSwapChain() const { return dxSwapChain_.get(); }

	// ビューと添付先からRenderTextureを取得する
	RenderTexture* GetRenderTexture(ViewType type, SVTarget target) const;

	// 深度SRVとGUI用レンダーターゲットのGPUハンドルを取得する
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

	// Descriptorヒープ(RTV/DSV/SRV)と深度リソースを初期化する
	void InitDescriptor(ID3D12Device8* device);
	// 各ビュー(Main/Debug)のMRTや深度SRV等を生成する
	void InitRenderTextrue(ID3D12Device8* device);
	// メッシュ/スプライト/スキニング等のレンダラを初期化する
	void InitRenderer(ID3D12Device8* device, DxShaderCompiler* shaderCompiler);

	// 各レンダラとライン/パーティクル描画をまとめて実行する
	void Renderers(ViewType type, bool enableMesh);

	// 指定MRTでレンダーターゲットの設定/クリア/ビューポート設定を行う
	void BeginRenderTarget(MultiRenderTexture* multiRenderTexture);
	// 指定MRTをCompute入力へ状態遷移する

	void EndRenderTarget(MultiRenderTexture* multiRenderTexture);

	// ビューごとのMRTを生成して登録する
	MultiRenderTexture* CreateViewMRT(ViewType type, ID3D12Device8* device);
	// 標準のアタッチメント(色/マスク)を追加する
	void AddDefaultAttachments(MultiRenderTexture* multiRenderTexture);
	// GUI用のコピー先テクスチャを生成する
	void CreateGuiRenderTexture(ID3D12Device8* device);
	// 深度バッファをSRVとして参照できるようにする
	void CreateDepthSRV();
};