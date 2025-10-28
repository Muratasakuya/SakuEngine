#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/Transform.h>
#include <Engine/Effect/Particle/Module/Base/ICPUParticleModule.h>
#include <Engine/Effect/Particle/Structures/ParticleEmitterStructures.h>

// imgui
#include <imgui.h>
// c++
#include <list>
// front
class Asset;

//============================================================================
//	ICPUParticleSpawnModule class
//	CPUパーティクル発生モジュール基底
//============================================================================
class ICPUParticleSpawnModule :
	public ICPUParticleModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ICPUParticleSpawnModule() = default;
	virtual ~ICPUParticleSpawnModule() = default;

	// 受け取ったパーティクルの発生処理を行う
	virtual void Execute(std::list<CPUParticle::ParticleData>& particles) = 0;

	virtual void UpdateEmitter() {}
	virtual void DrawEmitter() {}

	// editor
	void ImGuiRenderParam(bool hasTrailModule);
	void ImGuiPrimitiveParam();
	void ImGuiEmitParam();

	//--------- accessor -----------------------------------------------------

	void SetAsset(Asset* asset) { asset_ = asset; }
	void SetPrimitiveType(ParticlePrimitiveType type);
	// 親の設定
	void SetParent(bool isSet, const BaseTransform& parent);

	// データ共有
	void ShareCommonParam(ICPUParticleSpawnModule* other);

	float GetLifeTime() const { return lifeTime_.GetValue(); }
	const CPUParticle::TextureInfoForGPU& GetTextureInfo() const { return textureInfo_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Asset* asset_;

	// 全ての発生に置いて共通しているパラメータ
	// Emit
	ParticleValue<uint32_t> emitCount_; // 発生数
	ParticleValue<float> lifeTime_;     // 生存時間

	// 移動速度
	ParticleValue<float> moveSpeed_;

	// TextureInfo、ランダムがないのでそのまま渡す
	CPUParticle::TextureInfoForGPU textureInfo_;
	// トレイル
	ParticleCommon::TrailTextureInfoForGPU trailTextureInfo_;

	// Primtive
	ParticleCommon::PrimitiveData<false> primitive_;
	ParticlePlaneType planeType_;

	// editor
	const Color emitterLineColor_ = Color::Yellow(0.4f);
	std::string textureName_;
	std::string noiseTextureName_;
	// トレイル
	std::string trailTextureName_;
	std::string trailNoiseTextureName_;

	// 親設定
	const BaseTransform* parentTransform_ = nullptr;

	//--------- functions ----------------------------------------------------

	// init
	void InitCommonData();

	// helper
	void SetCommonData(CPUParticle::ParticleData& particle);

	// json
	void ToCommonJson(Json& data);
	void FromCommonJson(const Json& data);
private:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- functions ----------------------------------------------------

	// editor
	void DragAndDropTexture(bool isTrail);
};