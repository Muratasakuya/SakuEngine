#include "PostProcessCommandContext.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Core/ComputePostProcessor.h>
#include <Engine/Core/Graphics/PostProcess/PostProcessConfig.h>
#include <Engine/Core/Graphics/DxLib/DxUtils.h>

//============================================================================
//	PostProcessCommandContext classMethods
//============================================================================

void PostProcessCommandContext::Execute(PostProcessType type,
	ID3D12GraphicsCommandList* commandList, ComputePostProcessor* processor,
	const D3D12_GPU_DESCRIPTOR_HANDLE& inputTextureGPUHandle,
	const D3D12_GPU_DESCRIPTOR_HANDLE& inputMaskTextureGPUHandle) {

	UINT threadGroupCountX = DxUtils::RoundUp(static_cast<UINT>(processor->GetTextureSize().x), THREAD_POSTPROCESS_GROUP);
	UINT threadGroupCountY = DxUtils::RoundUp(static_cast<UINT>(processor->GetTextureSize().y), THREAD_POSTPROCESS_GROUP);

	// 共通設定
	commandList->SetComputeRootDescriptorTable(0, processor->GetUAVGPUHandle());
	commandList->SetComputeRootDescriptorTable(1, inputTextureGPUHandle);
	commandList->SetComputeRootDescriptorTable(2, inputMaskTextureGPUHandle);

	// typeごとに処理
	switch (type) {
	case PostProcessType::CopyTexture:
	case PostProcessType::Random:
	case PostProcessType::Bloom:
	case PostProcessType::Vignette:
	case PostProcessType::Grayscale:
	case PostProcessType::SepiaTone:
	case PostProcessType::BoxFilter:
	case PostProcessType::RadialBlur:
	case PostProcessType::VerticalBlur:
	case PostProcessType::GaussianFilter:
	case PostProcessType::HorizontalBlur:
	case PostProcessType::LuminanceBasedOutline:
	case PostProcessType::CRTDisplay:
	case PostProcessType::PlayerAfterImage:

		commandList->Dispatch(threadGroupCountX, threadGroupCountY, 1);
		break;
	case PostProcessType::Dissolve:
	case PostProcessType::DepthBasedOutline:
	case PostProcessType::Lut:
	case PostProcessType::Glitch:

		commandList->SetComputeRootDescriptorTable(3, processor->GetProcessTextureGPUHandle());
		commandList->Dispatch(threadGroupCountX, threadGroupCountY, 1);
		break;
	}
}