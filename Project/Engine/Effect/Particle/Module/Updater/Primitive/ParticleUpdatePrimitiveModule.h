#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleUpdateModule.h>
#include <Engine/Effect/Particle/Module/Updater/Primitive/Interface/IParticlePrimitiveUpdater.h>
#include <Engine/Effect/Particle/Structures/ParticleStructures.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	ParticleUpdatePrimitiveModule class
//	プリミティブ形状に基づいてパーティクルを更新するモジュール
//============================================================================
class ParticleUpdatePrimitiveModule :
	public ICPUParticleUpdateModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleUpdatePrimitiveModule() = default;
	~ParticleUpdatePrimitiveModule() = default;

	void Init() override;

	void Execute(CPUParticle::ParticleData& particle, float deltaTime) override;

	void ImGui() override;

	// json
	Json ToJson() override;
	void FromJson(const Json& data) override;

	//--------- accessor -----------------------------------------------------

	const char* GetName() const override { return "Primitive"; }

	//-------- registryID ----------------------------------------------------

	static constexpr ParticleUpdateModuleID ID = ParticleUpdateModuleID::Primitive;
	ParticleUpdateModuleID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 形状、particleの型によって処理を変更
	ParticlePrimitiveType type_;
	// 各形状の更新クラス配列
	std::array<std::unique_ptr<IParticlePrimitiveUpdater>,
		static_cast<uint32_t>(ParticlePrimitiveType::Count)> updaters_;

	EasingType easingType_;
};