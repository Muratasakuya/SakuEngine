#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Structures/ParticleStructures.h>
#include <Engine/Effect/Particle/Structures/ParticlePrimitiveStructures.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	IParticlePrimitiveUpdater class
//============================================================================
class IParticlePrimitiveUpdater {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IParticlePrimitiveUpdater() = default;
	virtual ~IParticlePrimitiveUpdater() = default;

	virtual void Init() = 0;

	virtual void Update(CPUParticle::ParticleData& particle, EasingType easingType) = 0;

	virtual void ImGui() = 0;

	// json
	virtual void FromJson(const Json& data) = 0;
	virtual void ToJson(Json& data) const = 0;

	//--------- accessor -----------------------------------------------------

	virtual ParticlePrimitiveType GetType() = 0;
};