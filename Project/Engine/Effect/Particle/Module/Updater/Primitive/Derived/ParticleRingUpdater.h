#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Updater/Primitive/Interface/BaseParticlePrimitiveUpdater.h>

//============================================================================
//	ParticleRingUpdater class
//	リング型のパーティクルを更新するクラス
//============================================================================
class ParticleRingUpdater :
	public BaseParticlePrimitiveUpdater<RingForGPU> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleRingUpdater() = default;
	~ParticleRingUpdater() = default;

	void Init() override;

	void Update(CPUParticle::ParticleData& particle, EasingType easingType) override;

	void ImGui() override;

	// json
	void FromJson(const Json& data) override;
	void ToJson(Json& data) const override;

	//--------- accessor -----------------------------------------------------

	ParticlePrimitiveType GetType() override { return ParticlePrimitiveType::Ring; }
};