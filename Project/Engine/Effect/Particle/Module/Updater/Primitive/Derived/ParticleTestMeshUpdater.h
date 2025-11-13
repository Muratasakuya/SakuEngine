#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Updater/Primitive/Interface/BaseParticlePrimitiveUpdater.h>

//============================================================================
//	ParticleTestMeshUpdater class
//	テッシュメッシュ型のパーティクルを更新するクラス
//============================================================================
class ParticleTestMeshUpdater :
	public BaseParticlePrimitiveUpdater<TestMeshForGPU> {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleTestMeshUpdater() = default;
	~ParticleTestMeshUpdater() = default;

	void Init() override;

	void Update(CPUParticle::ParticleData& particle, EasingType easingType) override;

	void ImGui() override;

	// json
	void FromJson(const Json& data) override;
	void ToJson(Json& data) const override;

	//--------- accessor -----------------------------------------------------

	ParticlePrimitiveType GetType() override { return ParticlePrimitiveType::TestMesh; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ParticlePlaneType planeType_;
};