#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Data/Base/BaseParticleGroup.h>
#include <Engine/Effect/Particle/Command/ParticleCommand.h>

// front
class Asset;

//============================================================================
//	GPUParticleGroup class
//	GPUのパーティクル処理をまとめたグループ
//============================================================================
class GPUParticleGroup :
	public BaseParticleGroup {
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structures ---------------------------------------------------

	// GPUに渡す親の情報
	struct ParentForGPU {

		Matrix4x4 parentMatrix;
		uint32_t aliveParent;
	};
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GPUParticleGroup() = default;
	~GPUParticleGroup() = default;

	// グループの作成
	void Create(ID3D12Device* device, Asset* asset, ParticlePrimitiveType primitiveType);
	void CreateFromJson(ID3D12Device* device, Asset* asset, const Json& data);

	// 発生間隔の更新
	void Update();

	// editor
	void ImGui(ID3D12Device* device);

	// json
	Json ToJson() const;
	void FromJson(const Json& data);

	//--------- accessor -----------------------------------------------------

	// 初期化済みフラグの設定、取得
	void SetIsInitialized(bool isInitialized) { isInitialized_ = isInitialized; }
	bool IsInitialized() const { return isInitialized_; }

	// 強制発生フラグの設定、取得
	void SetIsForcedEmit(bool emit);
	bool IsForcedEmit() const { return isForcedEmit_; }

	ParticlePrimitiveType GetPrimitiveType() const { return primitiveBuffer_.type; }
	ParticleEmitterShape GetEmitterShape() const { return emitter_.shape; }
	GPUParticle::UpdateType GetUpdateType() const { return updateType_; }
	BlendMode GetBlendMode() const { return blendMode_; }

	const std::string& GetTextureName() const { return textureName_; }
	const std::string& GetNoiseTextureName() const { return noiseTextureName_; }
	uint32_t GetEmitCount() const { return emitter_.common.count; }

	D3D12_GPU_VIRTUAL_ADDRESS GetEmitterShapeBufferAdress() const;
	const DxConstBuffer<ParentForGPU>& GetParentBuffer() const { return parentBuffer_; }
	const DxConstBuffer<GPUParticle::NoiseForGPU>& GetNoiseBuffer() const { return noiseBuffer_; }
	const DxConstBuffer<ParticleEmitterCommon>& GetEmitterCommonBuffer() const { return emitterBuffer_.common; }
	const DxStructuredBuffer<ParticleCommon::TransformForGPU>& GetTransformBuffer() const { return transformBuffer_; }
	const DxStructuredBuffer<GPUParticle::MaterialForGPU>& GetMaterialBuffer() const { return materialBuffer_; }
	const DxStructuredBuffer<GPUParticle::ParticleForGPU>& GetParticleBuffer() const { return particleBuffer_; }
	const DxStructuredBuffer<uint32_t>& GetFreeListIndexBuffer() const { return freeListIndexBuffer_; }
	const DxStructuredBuffer<uint32_t>& GetFreeListBuffer() const { return freeListBuffer_; }

	//---------- runtime -----------------------------------------------------

	// モジュールのコマンド適応
	void ApplyCommand(const ParticleCommand& command);

	//----------- emit -------------------------------------------------------

	// 一定間隔
	void FrequencyEmit();
	// 強制発生
	void Emit();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Asset* asset_;

	// 時間管理
	float frequency_;
	float frequencyTime_;
	// 初期化したかどうか
	bool isInitialized_;
	bool isForcedEmit_;

	// bufferに渡すデータ
	GPUParticle::NoiseForGPU noiseUpdate_;
	std::string noiseTextureName_;

	// buffers
	ParticleEmitterBufferData emitterBuffer_;
	DxConstBuffer<ParentForGPU> parentBuffer_;
	DxConstBuffer<GPUParticle::NoiseForGPU> noiseBuffer_;
	DxStructuredBuffer<GPUParticle::MaterialForGPU> materialBuffer_;
	DxStructuredBuffer<GPUParticle::ParticleForGPU> particleBuffer_;
	// freeList
	DxStructuredBuffer<uint32_t> freeListIndexBuffer_;
	DxStructuredBuffer<uint32_t> freeListBuffer_;

	// parameters
	float scalingValue_;

	// 更新の種類
	GPUParticle::UpdateType updateType_;

	//--------- functions ----------------------------------------------------

	// update
	void UpdateEmitter();
	void UpdateParent();
	void UpdateNoise();

	// editor
	void SelectEmitter(ID3D12Device* device);
};