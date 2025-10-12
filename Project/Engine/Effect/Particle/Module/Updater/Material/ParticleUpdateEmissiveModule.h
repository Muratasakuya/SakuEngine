#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	ParticleUpdateEmissiveModule class
//============================================================================
class ParticleUpdateEmissiveModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdateEmissiveModule() = default;
	~ParticleUpdateEmissiveModule() = default;

	void Init() override;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	const char* GetName() const override { return "Emissive"; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::Emissive;
	ParticleUpdateModuleID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 発光
	ParticleCommon::LerpValue<float> intencity_; // 発光度合い
	ParticleCommon::LerpValue<Vector3> color_;   // 色

	EasingType easingType_;
};