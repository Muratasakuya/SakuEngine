#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>
#include <Engine/Utility/Material/SerialUVScroll.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	ParticleUpdateNoiseUVModule class
//============================================================================
class ParticleUpdateNoiseUVModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdateNoiseUVModule() = default;
	~ParticleUpdateNoiseUVModule() = default;

	void Init() override;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	const char* GetName() const override { return "NoiseUV"; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::NoiseUV;
	ParticleUpdateModuleID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 更新の種類
	enum class UpdateType {

		Lerp,
		Scroll,
		Serial
	};

	//--------- variables ----------------------------------------------------

	// UV座標
	ParticleCommon::LerpValue<Vector3> translation_;
	ParticleCommon::LerpValue<float> rotation_;
	ParticleCommon::LerpValue<Vector3> scale_;

	// ピボット
	Vector2 pivot_;

	// スクロール加算値
	Vector2 scrollValue_;

	// 連番アニメーション
	SerialUVScroll serialScroll_;

	EasingType easing_;
	UpdateType updateType_;
};