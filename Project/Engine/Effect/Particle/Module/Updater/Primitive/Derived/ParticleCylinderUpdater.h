#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Updater/Primitive/Interface/BaseParticlePrimitiveUpdater.h>

//============================================================================
//	ParticleCylinderUpdater class
//	円柱型のパーティクルを更新するクラス
//============================================================================
class ParticleCylinderUpdater :
	public BaseParticlePrimitiveUpdater<CylinderForGPU> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleCylinderUpdater() = default;
	~ParticleCylinderUpdater() = default;

	void Init() override;

	void Update(CPUParticle::ParticleData& particle, EasingType easingType) override;

	void ImGui() override;

	// json
	void FromJson(const Json& data) override;
	void ToJson(Json& data) const override;

	//--------- accessor -----------------------------------------------------

	ParticlePrimitiveType GetType() override { return ParticlePrimitiveType::Cylinder; }
};