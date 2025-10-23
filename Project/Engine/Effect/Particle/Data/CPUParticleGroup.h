#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Data/Base/BaseParticleGroup.h>
#include <Engine/Effect/Particle/Phase/ParticlePhase.h>

//============================================================================
//	CPUParticleGroup class
//	CPUのパーティクル処理をまとめたグループ
//============================================================================
class CPUParticleGroup :
	public BaseParticleGroup {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	CPUParticleGroup() = default;
	~CPUParticleGroup() = default;

	// コピー禁止
	CPUParticleGroup(const CPUParticleGroup&) = delete;
	CPUParticleGroup& operator=(const CPUParticleGroup&) = delete;
	// ムーブ許可
	CPUParticleGroup(CPUParticleGroup&&) noexcept = default;
	CPUParticleGroup& operator=(CPUParticleGroup&&) noexcept = default;

	// グループの作成
	void Create(ID3D12Device* device, Asset* asset, ParticlePrimitiveType primitiveType);
	void CreateFromJson(ID3D12Device* device, Asset* asset, const Json& data, bool useGame);

	// 各パーティクルのフェーズを更新する
	void Update();

	// editor
	void ImGui();

	// json
	Json ToJson() const;
	void FromJson(const Json& data, Asset* asset);

	//--------- accessor -----------------------------------------------------

	ParticlePrimitiveType GetPrimitiveType() const { return primitiveBuffer_.type; }
	BlendMode GetBlendMode() const { return blendMode_; }
	uint32_t GetNumInstance() const { return numInstance_; }
	bool HasTrailModule() const;
	bool IsDrawParticle() const { return isDrawParticle_; }

	const DxStructuredBuffer<ParticleCommon::TransformForGPU>& GetTransformBuffer() const { return transformBuffer_; }
	const DxStructuredBuffer<ParticleCommon::TrailHeaderForGPU>& GetTrailHeaderBuffer() const { return trailHeaderBuffer_; }
	const DxStructuredBuffer<ParticleCommon::TrailVertexForGPU>& GetTrailVertexBuffer() const { return trailVertexBuffer_; }
	const DxStructuredBuffer<ParticleCommon::TrailTextureInfoForGPU>& GetTrailTextureInfoBuffer() const { return trailTextureInfoBuffer_; }

	const DxStructuredBuffer<CPUParticle::MaterialForGPU>& GetMaterialBuffer() const { return materialBuffer_; }
	const DxStructuredBuffer<CPUParticle::TextureInfoForGPU>& GetTextureInfoBuffer() const { return textureInfoBuffer_; }

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

	// インスタンス数
	uint32_t numInstance_;

	// フェーズ
	std::vector<std::unique_ptr<ParticlePhase>> phases_;

	// データ
	std::list<CPUParticle::ParticleData> particles_;
	// 転送データ
	std::vector<CPUParticle::MaterialForGPU> transferMaterials_;
	std::vector<CPUParticle::TextureInfoForGPU> transferTextureInfos_;
	std::vector<ParticleCommon::TransformForGPU> transferTransforms_;
	ParticleCommon::PrimitiveData<true> transferPrimitives_;
	// トレイル
	std::vector<ParticleCommon::TrailHeaderForGPU> transferTrailHeaders_;
	std::vector<ParticleCommon::TrailVertexForGPU> transferTrailVertices_;
	std::vector<ParticleCommon::TrailTextureInfoForGPU> transferTrailTextureInfos_;

	// buffers
	DxStructuredBuffer<CPUParticle::MaterialForGPU> materialBuffer_;
	DxStructuredBuffer<CPUParticle::TextureInfoForGPU> textureInfoBuffer_;

	// 描画情報
	bool isDrawParticle_ = true;
	BlendMode blendMode_;

	// editor
	int selectedPhase_ = -1;
	bool isSynchPhase_ = true;

	//--------- functions ----------------------------------------------------

	// update
	void UpdatePhase();
	void UpdateTransferData(uint32_t particleIndex,
		const CPUParticle::ParticleData& particle);
	void TransferBuffer();

	// helper
	void ResizeTransferData(uint32_t size);
	void AddPhase();
};