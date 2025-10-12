#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>

//============================================================================
//	ParticleUpdateLifeTimeModule enum class
//============================================================================

// 寿命が尽きた時の処理
enum class ParticleLifeEndMode {

	Advance, // 次のフェーズに進む
	Clamp,   // 最大時間で固定、次にも進まない
	Reset,   // 時間をリセットして再処理
	Kill     // 即削除
};

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