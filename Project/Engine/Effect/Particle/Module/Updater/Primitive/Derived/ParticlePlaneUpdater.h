#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Updater/Primitive/Interface/BaseParticlePrimitiveUpdater.h>

//============================================================================
//	ParticlePlaneUpdater class
//============================================================================
class ParticlePlaneUpdater :
	public BaseParticlePrimitiveUpdater<PlaneForGPU> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticlePlaneUpdater() = default;
	~ParticlePlaneUpdater() = default;

	void Init() override;

	void Update(CPUParticle::ParticleData& particle, EasingType easingType) override;

	void ImGui() override;

	// json
	void FromJson(const Json& data) override;
	void ToJson(Json& data) const override;

	//--------- accessor -----------------------------------------------------

	ParticlePrimitiveType GetType() override { return ParticlePrimitiveType::Plane; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ParticlePlaneType planeType_;
};