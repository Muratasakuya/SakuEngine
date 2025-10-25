#include "EffectCommandRouter.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/System/ParticleSystem.h>

//============================================================================
//	EffectCommandRouter classMethods
//============================================================================

void EffectCommandRouter::Send(ParticleSystem* system, const ParticleCommand& command) {

	// システムが無効なら何もしない
	if (!system) {
		return;
	}
	// システムにコマンドを送る
	system->ApplyCommand(command);
}