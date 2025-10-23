#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Pipeline/PipelineState.h>
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>
#include <Engine/Effect/Particle/Structures/ParticleEmitterStructures.h>
#include <Engine/Effect/Particle/Structures/ParticleStructures.h>

// c++
#include <array>
#include <cstdint>
// front
class Asset;
class SRVDescriptor;
class DxShaderCompiler;
class DxCommand;
class GPUParticleGroup;

//============================================================================
//	GPUParticleUpdater class
//	GPUパーティクルの発生、更新を行う
//============================================================================
class GPUParticleUpdater {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GPUParticleUpdater() = default;
	~GPUParticleUpdater() = default;

	// CS用のパイプライン初期化
	void Init(ID3D12Device8* device, Asset* asset, SRVDescriptor* srvDescriptor,
		DxShaderCompiler* shaderCompiler);

	// GPUパーティクルの更新
	void Update(GPUParticleGroup& group, DxCommand* dxCommand);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Asset* asset_;

	// それぞれのpipelineの数
	static const uint32_t kEmitterShapeCount = static_cast<uint32_t>(ParticleEmitterShape::Count);
	static const uint32_t kUpdateTypeCount = static_cast<uint32_t>(GPUParticle::UpdateType::Count);

	// 初期化
	std::unique_ptr<PipelineState> initPipeline_;
	// 発生
	std::array<std::unique_ptr<PipelineState>, kEmitterShapeCount> emitPipelines_;
	// 更新
	std::array<std::unique_ptr<PipelineState>, kUpdateTypeCount> updatePipelines_;

	// 共通buffer
	ParticleCommon::PerFrameForGPU perFrame_;
	DxConstBuffer<ParticleCommon::PerFrameForGPU> perFrameBuffer_;

	//--------- functions ----------------------------------------------------

	// init
	void InitPipelines(ID3D12Device8* device, SRVDescriptor* srvDescriptor,
		DxShaderCompiler* shaderCompiler);

	void BeginUpdate();

	// 初期化
	void DispatchInit(GPUParticleGroup& group, DxCommand* dxCommand);
	// 発生
	void DispatchEmit(GPUParticleGroup& group, DxCommand* dxCommand);
	// 更新
	void DispatchUpdate(const GPUParticleGroup& group, DxCommand* dxCommand);
};