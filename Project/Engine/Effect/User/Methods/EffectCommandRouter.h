#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Command/ParticleCommand.h>

// front
class ParticleSystem;

//============================================================================
//	EffectCommandRouter class
//	パーティクルシステムにコマンドを送る
//============================================================================
class EffectCommandRouter {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	EffectCommandRouter() = default;
	~EffectCommandRouter() = default;

	// システム全体にコマンドを送る
	static void Send(ParticleSystem* system, const ParticleCommand& command);
};