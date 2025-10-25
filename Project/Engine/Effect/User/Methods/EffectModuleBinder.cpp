#include "EffectModuleBinder.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/Methods/EffectCommandRouter.h>

//============================================================================
//	EffectModuleBinder classMethods
//============================================================================

void EffectModuleBinder::ApplyPreEmit(ParticleSystem* system,
	const EffectModuleSetting& module, const Vector3& worldPos) {

	// 発生位置を設定するコマンドを作成して送る
	ParticleCommand command = MakeCommand<Vector3>(
		ParticleCommandTarget::Spawner, ParticleCommandID::SetTranslation, worldPos);
	EffectCommandRouter::Send(system, command);

	// スケールの設定
	// 発生モジュール
	if (module.spawnerScaleEnable) {

		command = MakeCommand<float>(
			ParticleCommandTarget::Spawner, ParticleCommandID::Scaling, module.spawnerScaleValue);
		EffectCommandRouter::Send(system, command);
	}
	// 更新モジュール
	if (module.updaterScaleEnable) {
		
		command = MakeCommand<float>(
			ParticleCommandTarget::Updater, ParticleCommandID::Scaling, module.updaterScaleValue);
		EffectCommandRouter::Send(system, command);
	}
}