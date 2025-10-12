#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>

//============================================================================
//	ParticleUpdateScaleModule class
//============================================================================
class ParticleUpdateScaleModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdateScaleModule() = default;
	~ParticleUpdateScaleModule() = default;

	void Init() override;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	bool SetCommand(const ParticleCommand& command) override;

	const char* GetName() const override { return "Scale"; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::Scale;
	ParticleUpdateModuleID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ParticleCommon::LerpValue<Vector3> scale_;
	Vector3 scalingValue_;

	EasingType easing_;
};