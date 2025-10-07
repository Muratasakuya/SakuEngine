#include "PostProcessPipeline.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Helper/Algorithm.h>

//============================================================================
//	PostProcessPipeline classMethods
//============================================================================

void PostProcessPipeline::Init(ID3D12Device8* device, SRVDescriptor* srvDescriptor,
	DxShaderCompiler* shaderCompiler) {

	device_ = nullptr;
	device_ = device;
	srvDescriptor_ = nullptr;
	srvDescriptor_ = srvDescriptor;
	shaderCompiler_ = nullptr;
	shaderCompiler_ = shaderCompiler;

	// shaderDataFileName
	fileNames_ = {
		"CopySceneTexture.json",
		"Bloom.json",
		"HorizontalBlur.json",
		"VerticalBlur.json",
		"RadialBlur.json",
		"GaussianFilter.json",
		"BoxFilter.json",
		"Dissolve.json",
		"Random.json",
		"Vignette.json",
		"Grayscale.json",
		"SepiaTone.json",
		"LuminanceBasedOutline.json",
		"DepthBasedOutline.json",
		"Lut.json",
		"Glitch.json",
		"CRTDisplay.json",
	};

	for (const uint32_t& type : Algorithm::GetEnumArray(PostProcessType::Count)) {

		pipelines_[type] = std::make_unique<PipelineState>();
	}

	// copy用は最初に作成する
	Create(PostProcessType::CopyTexture);
}

void PostProcessPipeline::Create(PostProcessType type) {

	// pipelineの作成
	uint32_t processIndex = static_cast<uint32_t>(type);
	pipelines_[processIndex]->Create(fileNames_[processIndex], device_, srvDescriptor_, shaderCompiler_);
}

void PostProcessPipeline::SetPipeline(ID3D12GraphicsCommandList* commandList, PostProcessType type) {

	commandList->SetComputeRootSignature(pipelines_[static_cast<uint32_t>(type)]->GetRootSignature());
	commandList->SetPipelineState(pipelines_[static_cast<uint32_t>(type)]->GetComputePipeline());
}