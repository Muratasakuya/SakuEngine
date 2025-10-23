#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/Base/ICPUParticleModule.h>
#include <Engine/Effect/Particle/Module/Base/ParticleLoopableModule.h>

//============================================================================
//	ICPUParticleUpdateModule class
//	CPUパーティクル更新モジュール基底
//============================================================================
class ICPUParticleUpdateModule :
	public ICPUParticleModule,
	public ParticleLoopableModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ICPUParticleUpdateModule() = default;
	~ICPUParticleUpdateModule() = default;

	void Init() override {}

	// 受け取ったパーティクルの更新処理を行う
	virtual void Execute(CPUParticle::ParticleData& particle, float deltaTime) = 0;

	//--------- accessor -----------------------------------------------------

	virtual ParticleUpdateModuleID GetID() const = 0;
};