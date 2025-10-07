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
//============================================================================
class PostProcessPipeline {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PostProcessPipeline() = default;
	~PostProcessPipeline() = default;

	void Init(ID3D12Device8* device, SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler);

	void Create(PostProcessType type);

	//--------- accessor -----------------------------------------------------

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