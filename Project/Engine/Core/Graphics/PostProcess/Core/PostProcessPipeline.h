#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Pipeline/PipelineState.h>
#include <Engine/Core/Graphics/PostProcess/PostProcessType.h>

// c++
#include <memory>
#include <array>
// front
class SRVDescriptor;
class DxShaderCompiler;

//============================================================================
//	PostProcessPipeline class
//	各ポストプロセスのComputeパイプラインを生成/保持し、必要に応じて切り替える。
//============================================================================
class PostProcessPipeline {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PostProcessPipeline() = default;
	~PostProcessPipeline() = default;

	// 依存(デバイス/ディスクリプタ/コンパイラ)を記録し、コピー用PSOを先に用意する
	void Init(ID3D12Device8* device, SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler);
	// 指定PostProcessTypeのComputeパイプラインを作成する
	void Create(PostProcessType type);

	//--------- accessor -----------------------------------------------------

	// 指定タイプのCompute RootSig/PSOをコマンドリストへセットする
	void SetPipeline(ID3D12GraphicsCommandList* commandList, PostProcessType type);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ID3D12Device8* device_;
	SRVDescriptor* srvDescriptor_;
	DxShaderCompiler* shaderCompiler_;

	// pipelineごとのfileの名前
	std::vector<std::string> fileNames_;

	std::array<std::unique_ptr<PipelineState>, kPostProcessCount> pipelines_;
};