#include "GPUParticleUpdater.h"

//============================================================================
//	GPUParticleUpdater classMethods
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Effect/Particle/Data/GPUParticleGroup.h>
#include <Engine/Effect/Particle/ParticleConfig.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	GPUParticleUpdater classMethods
//============================================================================

void GPUParticleUpdater::Init(ID3D12Device8* device, Asset* asset,
	SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler) {

	asset_ = nullptr;
	asset_ = asset;

	// 初期化
	perFrame_.time = 0.0f;
	perFrame_.deltaTime = 0.0f;
	// buffer作成
	perFrameBuffer_.CreateBuffer(device);

	// 各pipeline初期化
	InitPipelines(device, srvDescriptor, shaderCompiler);
}

void GPUParticleUpdater::Update(GPUParticleGroup& group, DxCommand* dxCommand) {

	// 時間の更新
	BeginUpdate();

	// 各GPU処理
	// 初期化
	DispatchInit(group, dxCommand);
	// 発生
	DispatchEmit(group, dxCommand);
	// 更新
	DispatchUpdate(group, dxCommand);
}

void GPUParticleUpdater::BeginUpdate() {

	// 時間の更新
	perFrame_.deltaTime = GameTimer::GetDeltaTime();
	perFrame_.time += perFrame_.deltaTime;

	// buffer転送
	perFrameBuffer_.TransferData(perFrame_);
}

void GPUParticleUpdater::InitPipelines(ID3D12Device8* device,
	SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler) {

	initPipeline_ = std::make_unique<PipelineState>();
	initPipeline_->Create("InitParticle.json", device, srvDescriptor, shaderCompiler);

	for (uint32_t index = 0; index < kUpdateTypeCount; ++index) {

		// 更新の種類の名前を取得
		const char* typeName = EnumAdapter<GPUParticle::UpdateType>::GetEnumName(index);
		std::string jsonFile = std::string(typeName) + "UpdateParticle.json";

		auto& pipeline = updatePipelines_[index];
		pipeline = std::make_unique<PipelineState>();
		pipeline->Create(jsonFile, device, srvDescriptor, shaderCompiler);
	}

	for (uint32_t index = 0; index < kEmitterShapeCount; ++index) {

		// 形状の名前を取得
		const char* shapeName = EnumAdapter<ParticleEmitterShape>::GetEnumName(index);
		std::string jsonFile = "Emit" + std::string(shapeName) + "Particle.json";

		auto& pipeline = emitPipelines_[index];
		pipeline = std::make_unique<PipelineState>();
		pipeline->Create(jsonFile, device, srvDescriptor, shaderCompiler);
	}
}

void GPUParticleUpdater::DispatchInit(GPUParticleGroup& group, DxCommand* dxCommand) {

	// 初期化済みの場合は処理しない
	if (group.IsInitialized()) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = dxCommand->GetCommandList();

	// 初期化用のpipeline設定
	commandList->SetComputeRootSignature(initPipeline_->GetRootSignature());
	commandList->SetPipelineState(initPipeline_->GetComputePipeline());

	// particle
	commandList->SetComputeRootUnorderedAccessView(0, group.GetParticleBuffer().GetResource()->GetGPUVirtualAddress());
	// transform
	commandList->SetComputeRootUnorderedAccessView(1, group.GetTransformBuffer().GetResource()->GetGPUVirtualAddress());
	// material
	commandList->SetComputeRootUnorderedAccessView(2, group.GetMaterialBuffer().GetResource()->GetGPUVirtualAddress());
	// freeListIndex
	commandList->SetComputeRootUnorderedAccessView(3, group.GetFreeListIndexBuffer().GetResource()->GetGPUVirtualAddress());
	// freeList
	commandList->SetComputeRootUnorderedAccessView(4, group.GetFreeListBuffer().GetResource()->GetGPUVirtualAddress());

	// 実行
	commandList->Dispatch(1, 1, 1);

	// 各バッファのUAVバリア
	dxCommand->UAVBarrierAll();

	// 初期化済み
	group.SetIsInitialized(true);
}

void GPUParticleUpdater::DispatchEmit(GPUParticleGroup& group, DxCommand* dxCommand) {

	if (group.GetEmitCount() == 0) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = dxCommand->GetCommandList();
	const uint32_t shapeIndex = static_cast<uint32_t>(group.GetEmitterShape());

	// 発生用のpipeline設定
	commandList->SetComputeRootSignature(emitPipelines_[shapeIndex]->GetRootSignature());
	commandList->SetPipelineState(emitPipelines_[shapeIndex]->GetComputePipeline());

	// particle
	commandList->SetComputeRootUnorderedAccessView(0, group.GetParticleBuffer().GetResource()->GetGPUVirtualAddress());
	// transform
	commandList->SetComputeRootUnorderedAccessView(1, group.GetTransformBuffer().GetResource()->GetGPUVirtualAddress());
	// material
	commandList->SetComputeRootUnorderedAccessView(2, group.GetMaterialBuffer().GetResource()->GetGPUVirtualAddress());
	// freeListIndex
	commandList->SetComputeRootUnorderedAccessView(3, group.GetFreeListIndexBuffer().GetResource()->GetGPUVirtualAddress());
	// freeList
	commandList->SetComputeRootUnorderedAccessView(4, group.GetFreeListBuffer().GetResource()->GetGPUVirtualAddress());
	// emitterCommon
	commandList->SetComputeRootConstantBufferView(5, group.GetEmitterCommonBuffer().GetResource()->GetGPUVirtualAddress());
	// emitterShape
	commandList->SetComputeRootConstantBufferView(6, group.GetEmitterShapeBufferAdress());
	// perFrame
	commandList->SetComputeRootConstantBufferView(7, perFrameBuffer_.GetResource()->GetGPUVirtualAddress());

	// 実行
	commandList->Dispatch(DxUtils::RoundUp(group.GetEmitCount(), THREAD_EMIT_GROUP), 1, 1);

	// 各バッファのUAVバリア
	dxCommand->UAVBarrierAll();

	// 強制的に発生させていた場合は連続で発生させないためにfalseにする
	if (group.IsForcedEmit()) {

		group.SetIsForcedEmit(false);
	}
}

void GPUParticleUpdater::DispatchUpdate(const GPUParticleGroup& group, DxCommand* dxCommand) {

	ID3D12GraphicsCommandList6* commandList = dxCommand->GetCommandList();
	const uint32_t typeIndex = static_cast<uint32_t>(group.GetUpdateType());

	// 更新用のpipeline設定
	commandList->SetComputeRootSignature(updatePipelines_[typeIndex]->GetRootSignature());
	commandList->SetPipelineState(updatePipelines_[typeIndex]->GetComputePipeline());

	// particle
	commandList->SetComputeRootUnorderedAccessView(0, group.GetParticleBuffer().GetResource()->GetGPUVirtualAddress());
	// transform
	commandList->SetComputeRootUnorderedAccessView(1, group.GetTransformBuffer().GetResource()->GetGPUVirtualAddress());
	// material
	commandList->SetComputeRootUnorderedAccessView(2, group.GetMaterialBuffer().GetResource()->GetGPUVirtualAddress());
	// freeListIndex
	commandList->SetComputeRootUnorderedAccessView(3, group.GetFreeListIndexBuffer().GetResource()->GetGPUVirtualAddress());
	// freeList
	commandList->SetComputeRootUnorderedAccessView(4, group.GetFreeListBuffer().GetResource()->GetGPUVirtualAddress());
	// perFrame
	commandList->SetComputeRootConstantBufferView(5, perFrameBuffer_.GetResource()->GetGPUVirtualAddress());
	// perFrame
	commandList->SetComputeRootConstantBufferView(6, group.GetParentBuffer().GetResource()->GetGPUVirtualAddress());

	// ノイズを使用する場合の設定
	if (typeIndex == static_cast<uint32_t>(GPUParticle::UpdateType::Noise)) {

		// noise
		commandList->SetComputeRootConstantBufferView(7, group.GetNoiseBuffer().GetResource()->GetGPUVirtualAddress());
		// noiseTexture
		commandList->SetComputeRootDescriptorTable(8, asset_->GetGPUHandle(group.GetNoiseTextureName()));
	}

	// 実行
	commandList->Dispatch(DxUtils::RoundUp(kMaxGPUParticles, THREAD_UPDATE_GROUP), 1, 1);

	//============================================================================
	// バリア遷移処理

	// ComputeShader -> MeshShader
	dxCommand->TransitionBarriers({ group.GetTransformBuffer().GetResource() },
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	// ComputeShader -> PixelShader
	dxCommand->TransitionBarriers({ group.GetMaterialBuffer().GetResource() },
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}