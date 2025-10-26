#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/Transform.h>
#include <Engine/Effect/Particle/Structures/ParticleStructures.h>
#include <Engine/Effect/Particle/Structures/ParticleEmitterStructures.h>

// front
class SceneView;

//============================================================================
//	BaseParticleGroup class
//	パーティクルの共通設定、バッファ管理
//============================================================================
class BaseParticleGroup {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BaseParticleGroup() = default;
	virtual ~BaseParticleGroup() = default;

	//--------- accessor -----------------------------------------------------

	// シーンビューの設定
	void SetSceneView(SceneView* sceneView) { sceneView_ = sceneView; }

	// 親の設定
	void SetParent(bool isSet, const BaseTransform& parent);

	D3D12_GPU_VIRTUAL_ADDRESS GetPrimitiveBufferAdress() const;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	SceneView* sceneView_;

	// ゲームで使用するか
	bool useGame_;
	// ゲームで使用する際のパーティクル最大数(最大数作る必要がないものがあるため)
	int gameMaxParticleCount_;
	// バッファ作成した数
	uint32_t createParticleInstanceCount_;
	uint32_t createTrailInstanceCount_;

	// emitter
	Vector3 emitterRotation_;     // 回転(すべて共通)
	std::optional<Matrix4x4> setRotationMatrix_;
	ParticleEmitterData emitter_; // 各形状

	// commonBuffers
	// 形状
	ParticleCommon::PrimitiveBufferData primitiveBuffer_;
	// トランスフォーム
	DxStructuredBuffer<ParticleCommon::TransformForGPU> transformBuffer_;
	// トレイル
	DxStructuredBuffer<ParticleCommon::TrailHeaderForGPU> trailHeaderBuffer_;
	DxStructuredBuffer<ParticleCommon::TrailVertexForGPU> trailVertexBuffer_;
	DxStructuredBuffer<ParticleCommon::TrailTextureInfoForGPU> trailTextureInfoBuffer_;

	// 描画情報
	BlendMode blendMode_;
	std::string textureName_;

	// 親設定
	const BaseTransform* parentTransform_ = nullptr;

	//--------- functions ----------------------------------------------------

	// create
	void CreatePrimitiveBuffer(ID3D12Device* device,
		ParticlePrimitiveType primitiveType, uint32_t maxParticle);
	void CreateTrailBuffer(ID3D12Device* device,
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