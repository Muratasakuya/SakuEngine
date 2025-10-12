#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/Transform.h>
#include <Engine/Effect/Particle/Structures/ParticleStructures.h>
#include <Engine/Effect/Particle/Structures/ParticleEmitterStructures.h>

//============================================================================
//	BaseParticleGroup class
//============================================================================
class BaseParticleGroup {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BaseParticleGroup() = default;
	virtual ~BaseParticleGroup() = default;

	//--------- accessor -----------------------------------------------------

	// 親の設定
	void SetParent(bool isSet, const BaseTransform& parent);

	D3D12_GPU_VIRTUAL_ADDRESS GetPrimitiveBufferAdress() const;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// ゲームで使用するか
	bool useGame_;
	// ゲームで使用する際のパーティクル最大数(最大数作る必要がないものがあるため)
	int gameMaxParticleCount_;

	// emitter
	Vector3 emitterRotation_;     // 回転(すべて共通)
	std::optional<Matrix4x4> setRotationMatrix_;
	ParticleEmitterData emitter_; // 各形状

	// commonBuffers
	ParticleCommon::PrimitiveBufferData primitiveBuffer_;
	DxStructuredBuffer<ParticleCommon::TransformForGPU> transformBuffer_;

	// 描画情報
	BlendMode blendMode_;
	std::string textureName_;

	// 親設定
	const BaseTransform* parentTransform_ = nullptr;

	//--------- functions ----------------------------------------------------

	// create
	void CreatePrimitiveBuffer(ID3D12Device* device,
		ParticlePrimitiveType primitiveType, uint32_t maxParticle);

	// emitter
	void DrawEmitter();
	void EditEmitter();

	// editor
	bool ImGuiParent();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// editor
	std::vector<uint32_t> parentIDs_;
	std::vector<std::string> parentNames_;
};