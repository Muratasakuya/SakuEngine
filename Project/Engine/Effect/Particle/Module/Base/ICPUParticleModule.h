#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Structures/ParticleStructures.h>
#include <Engine/Effect/Particle/Command/ParticleCommand.h>

// imgui
#include <imgui.h>

//============================================================================
//	ICPUParticleModule class
//	CPUパーティクルモジュールの基底クラス
//============================================================================
class ICPUParticleModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ICPUParticleModule() = default;
	virtual ~ICPUParticleModule() = default;

	virtual void Init() = 0;

	virtual void ImGui() = 0;

	// json
	virtual Json ToJson() = 0;
	virtual void FromJson(const Json& data) = 0;

	//--------- accessor -----------------------------------------------------

	virtual void SetCommand([[maybe_unused]] const ParticleCommand& command) {}

	virtual const char* GetName() const = 0;
};