#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Pipeline/PipelineState.h>
#include <Engine/Core/Graphics/Raytracing/RaytracingPipeline.h>
#include <Engine/Core/Graphics/Raytracing/RaytracingScene.h>

// c++
#include <memory>
#include <ranges>
// front
class SRVDescriptor;
class DxCommand;
class RenderTexture;
class DepthTexture;
class SceneConstBuffer;

//============================================================================
//	MeshRenderer class
//	メッシュ描画の中核。メッシュ/スカイボックスのパイプライン、RayTracing(TLAS/BLAS)更新、描画実行を担う。
//============================================================================
class MeshRenderer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	MeshRenderer() = default;
	~MeshRenderer() = default;

	// 必要なパイプライン(メッシュ/スカイボックス)とレイトレシーンを初期化
	void Init(ID3D12Device8* device, DxShaderCompiler* shaderCompiler, SRVDescriptor* srvDescriptor);

	// オブジェクト情報からBLAS/TLASを再構築する
	void UpdateRayScene(DxCommand* dxCommand);

	// シーンバッファ/テクスチャ群/RT資源をセットしメッシュ描画を実行
	void Rendering(bool debugEnable, SceneConstBuffer* sceneBuffer, DxCommand* dxCommand);

	//--------- accessor -----------------------------------------------------

	// TLASのリソース(GPUVA参照用)を取得
	ID3D12Resource* GetTLASResource() const { return rayScene_->GetTLASResource(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	SRVDescriptor* srvDescriptor_;

	// main
	std::unique_ptr<PipelineState> meshShaderPipeline_;
	// skybox
	std::unique_ptr<PipelineState> skyboxPipeline_;

	// raytracing
	std::unique_ptr<RaytracingScene> rayScene_;

	//--------- functions ----------------------------------------------------

	// スキンドメッシュの頂点バッファを適切な状態に遷移(描画前)
	void BeginSkinnedTransition(bool debugEnable, uint32_t meshIndex, IMesh* mesh, DxCommand* dxCommand);
	// スキンドメッシュの頂点バッファを元の状態に戻す(描画後)
	void EndSkinnedTransition(bool debugEnable, uint32_t meshIndex, IMesh* mesh, DxCommand* dxCommand);
};