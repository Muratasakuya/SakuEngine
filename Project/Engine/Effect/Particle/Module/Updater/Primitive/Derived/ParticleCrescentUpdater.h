#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Updater/Primitive/Interface/BaseParticlePrimitiveUpdater.h>

//============================================================================
//	ParticleCrescentUpdater class
//============================================================================
class ParticleCrescentUpdater :
	public BaseParticlePrimitiveUpdater<CrescentForGPU> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleCrescentUpdater() = default;
	~ParticleCrescentUpdater() = default;

	void Init() override;

	void Update(CPUParticle::ParticleData& particle, EasingType easingType) override;

	void ImGui() override;

	// json
	void FromJson(const Json& data) override;
	void ToJson(Json& data) const override;

	//--------- accessor -----------------------------------------------------

	ParticlePrimitiveType GetType() override { return ParticlePrimitiveType::Crescent; }
};