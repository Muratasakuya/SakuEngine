#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>

//============================================================================
//	ParticleUpdateGravityModule class
//============================================================================
class ParticleUpdateGravityModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdateGravityModule() = default;
	~ParticleUpdateGravityModule() = default;

	void Init() override;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	const char* GetName() const override { return "Gravity"; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::Gravity;
	ParticleUpdateModuleID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	bool reflectGround_; // 地面に反射するか

	// 反射
	float reflectGroundY_; // 地面のY座標
	float restitution_;    // 強さ

	// 重力
	float strength_;    // 強さ
	Vector3 direction_; // 方向
};