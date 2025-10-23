#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>
#include <Engine/Effect/Particle/Command/ParticleCommand.h>

//============================================================================
//	ParticleUpdateLifeTimeModule class
//============================================================================
class ParticleUpdateLifeTimeModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdateLifeTimeModule() = default;
	~ParticleUpdateLifeTimeModule() = default;

	void Init() override;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	void SetCommand(const ParticleCommand& command) override;

	const char* GetName() const override { return "LifeTime"; }

	ParticleLifeEndMode GetEndMode() const { return endMode_; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::LifeTime;
	ParticleUpdateModuleID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// デフォルトで次のフェーズに進む
	ParticleLifeEndMode endMode_ = ParticleLifeEndMode::Advance;
};