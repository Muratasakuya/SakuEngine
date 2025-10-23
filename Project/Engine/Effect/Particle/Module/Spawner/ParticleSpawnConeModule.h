#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleSpawnModule.h>

//============================================================================
//	ParticleSpawnConeModule class
//	コーン型発生モジュール
//============================================================================
class ParticleSpawnConeModule :
	public ICPUParticleSpawnModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleSpawnConeModule() = default;
	~ParticleSpawnConeModule() = default;

	void Init() override;

	void Execute(std::list<CPUParticle::ParticleData>& particles) override;

	void UpdateEmitter() override;
	void DrawEmitter() override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	void SetCommand(const ParticleCommand& command) override;

	const char* GetName() const override { return "SpawnCone"; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleSpawnModuleID ID = ParticleSpawnModuleID::Cone;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Vector3 emitterRotation_;
	ParticleEmitterCone emitter_;

	//--------- functions ----------------------------------------------------

	// コーンの面上の点を取得
	Vector3 GetFacePoint(float radius, float height) const;
};