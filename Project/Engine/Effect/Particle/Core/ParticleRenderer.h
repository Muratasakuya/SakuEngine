#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Pipeline/PipelineState.h>
#include <Engine/Effect/Particle/Structures/ParticlePrimitiveStructures.h>
#include <Engine/Effect/Particle/Structures/ParticleStructures.h>

// c++
#include <array>
// front
class Asset;
class SRVDescriptor;
class DxShaderCompiler;
class DxCommand;
class SceneConstBuffer;
class GPUParticleGroup;
class CPUParticleGroup;

//============================================================================
//	ParticleRenderer class
//============================================================================
class ParticleRenderer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleRenderer() = default;
	~ParticleRenderer() = default;

	void Init(ID3D12Device8* device, Asset* asset,
		SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler);

	// GPU
	void Rendering(bool debugEnable, const GPUParticleGroup& group,
		SceneConstBuffer* sceneBuffer, DxCommand* dxCommand);

	// CPU
	void Rendering(bool debugEnable, const CPUParticleGroup& group,
		SceneConstBuffer* sceneBuffer, DxCommand* dxCommand);
	void RenderingTrail(bool debugEnable, const CPUParticleGroup& group,
		SceneConstBuffer* sceneBuffer, DxCommand* dxCommand);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 描画モード
	enum class RenderMode {

		None,  // 通常
		Trail, // トレイル
	};

	//--------- variables ----------------------------------------------------

	Asset* asset_;
	SRVDescriptor* srvDescriptor_;

	static const uint32_t kPrimitiveCount = static_cast<uint32_t>(ParticlePrimitiveType::Count);
	static const uint32_t kParticleTypeCount = static_cast<uint32_t>(ParticleType::Count);

	// 描画パイプライン
	// RenderModeMap: [ParticleType][PrimitiveType]
	std::unordered_map<RenderMode, std::array<std::array<
		std::unique_ptr<PipelineState>, kPrimitiveCount>, kParticleTypeCount>> pipelines_;

	//--------- functions ----------------------------------------------------

	// init
	void InitPipelines(ID3D12Device8* device, SRVDescriptor* srvDescriptor,
		DxShaderCompiler* shaderCompiler);

	// helper
	void SetPipeline(RenderMode mode, uint32_t typeIndex, uint32_t primitiveIndex,
		ID3D12GraphicsCommandList* commandList, BlendMode blendMode);
	void ToCompute(bool debugEnable, const GPUParticleGroup& group, DxCommand* dxCommand);
};