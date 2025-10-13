#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>

//============================================================================
//	ParticleUpdateNoiseForceModule class
//============================================================================
class ParticleUpdateNoiseForceModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdateNoiseForceModule() = default;
	~ParticleUpdateNoiseForceModule() = default;

	void Init() override;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	const char* GetName() const override { return "NoiseForce"; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::NoiseForce;
	ParticleUpdateModuleID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// ノイズの種類
	enum class NoiseMode {

		Gradient,
		Curl,
		Offset,
	};

	//--------- variables ----------------------------------------------------

	NoiseMode mode_ = NoiseMode::Curl;
	int octaves_;
	float frequency_; // 空間周波数
	float timeScale_; // 時間の進み
	float strength_;  // 速度へ加える力の強さ
	float damping_;   // 速度減衰
	uint32_t seed_;

	// オフセット用
	bool anchorToSpawn_ = true;
	Vector3 offsetAmp_;

	//--------- functions ----------------------------------------------------

	// helper
	float Noise3(const Vector3& p, uint32_t s) const;      // ノイズ生成
	float FBm(const Vector3& p, uint32_t s) const;         // オクターブ合成
	Vector3 GradNoise(const Vector3& p, uint32_t s) const; // ∇fBm
	Vector3 CurlNoise(const Vector3& p) const;             // ∇×(fBm1,fBm2,fBm3)
};