#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>

// front
class SceneView;

//============================================================================
//	ParticleUpdateTrailModule class
//============================================================================
class ParticleUpdateTrailModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdateTrailModule() = default;
	~ParticleUpdateTrailModule() = default;

	void Init() override;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;
	void BuildTransferData(uint32_t particleIndex, const CPUParticle::ParticleData& particle,
		std::vector<ParticleCommon::TrailHeaderForGPU>& transferTrailHeaders,
		std::vector<ParticleCommon::TrailVertexForGPU>& transferTrailVertices,
		const SceneView* sceneView);

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson([[maybe_unused]] const Json& data) override;

	//--------- accessor -----------------------------------------------------

	const char* GetName() const override { return "Trail"; }
	const ParticleCommon::TrailParam& GetParam() const { return param_; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::Trail;
	ParticleUpdateModuleID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ParticleCommon::TrailParam param_;
};