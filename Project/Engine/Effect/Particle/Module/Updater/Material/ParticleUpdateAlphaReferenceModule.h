#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	ParticleUpdateAlphaReferenceModule class
//============================================================================
class ParticleUpdateAlphaReferenceModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdateAlphaReferenceModule() = default;
	~ParticleUpdateAlphaReferenceModule() = default;

	void Init() override;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	const char* GetName() const override { return "AlphaReference"; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::AlphaReference;
	ParticleUpdateModuleID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// ノイズを使用するか
	bool useNoiseTexture_;

	// 棄却値
	ParticleCommon::LerpValue<float> color_; // 色
	ParticleCommon::LerpValue<float> noise_; // ノイズ

	EasingType easing_;
};