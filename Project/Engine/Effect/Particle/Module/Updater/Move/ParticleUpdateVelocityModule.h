#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>
#include <Engine/Utility/Curve/CurveValue.h>

//============================================================================
//	ParticleUpdateVelocityModule class
//============================================================================
class ParticleUpdateVelocityModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdateVelocityModule() = default;
	~ParticleUpdateVelocityModule() = default;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;

	void ImGui() override;

	// json
	Json ToJson() override { return Json(); };
	void FromJson([[maybe_unused]] const Json& data) override {};

	//--------- accessor -----------------------------------------------------

	const char* GetName() const override { return "Velocity"; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::Velocity;
	ParticleUpdateModuleID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// カーブテスト
	CurveValue<float> curveFloat_ = CurveValue<float>("Float");
	CurveValue<Vector2> curveVec2_ = CurveValue<Vector2>("Vector2");
	CurveValue<Vector3> curveVec3_ = CurveValue<Vector3>("Vector3");
	CurveValue<Color> curveColor_ = CurveValue<Color>("Color");
};