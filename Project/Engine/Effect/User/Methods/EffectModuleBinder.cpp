#include "EffectModuleBinder.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/Methods/EffectCommandRouter.h>

//============================================================================
//	EffectModuleBinder classMethods
//============================================================================

void EffectModuleBinder::ApplyPreEmit(ParticleSystem* system,
	const EffectModuleSetting& module, const Vector3& worldPos, const EffectCommandContext& context) {

	// 無効な場合は処理しない
	if (!system) {
		return;
	}

	// 位置
	if (module.sendSpawnerTranslation) {

		// 発生位置
		Vector3 pos = worldPos + module.spawnPos;
		if (module.posOption == EffectPosOption::ApplyNodeRotation) {

			// 発生回転を座標へ適用
			const Quaternion rotation = Quaternion::EulerToQuaternion(module.spawnRotate);
			pos = rotation * module.spawnPos + worldPos;
		}

		// 位置コマンド送信
		ParticleCommand command = MakeCommand<Vector3>(ParticleCommandTarget::Spawner,
			ParticleCommandID::SetTranslation, pos);
		EffectCommandRouter::Send(system, command);
	}

	// 発生回転
	if (module.sendSpawnerRotation && module.spawnRotateOption == EffectRotateOption::UseSpawnEuler) {

		// 回転コマンド送信
		ParticleCommand command = MakeCommand<Vector3>(ParticleCommandTarget::Spawner,
			ParticleCommandID::SetRotation, module.spawnRotate);
		EffectCommandRouter::Send(system, command);
	}
	// カメラの方を発生モジュールが向くようにする
	if (module.spawnRotateOption == EffectRotateOption::BillboardCamera && context.billboardProvider) {

		// ビルボード行列を取得して送信
		Matrix4x4 billboard = context.billboardProvider();
		ParticleCommand command = MakeCommand<Matrix4x4>(ParticleCommandTarget::Spawner,
			ParticleCommandID::SetBillboardRotation, billboard);
		EffectCommandRouter::Send(system, command);
	}

	// スケール
	// 発生モジュール
	if (module.spawnerScaleEnable) {

		// スケールコマンド送信
		ParticleCommand command = MakeCommand<float>(ParticleCommandTarget::Spawner,
			ParticleCommandID::Scaling, module.spawnerScaleValue);
		EffectCommandRouter::Send(system, command);
	}
	// 更新モジュール
	if (module.updaterScaleEnable) {

		// スケールコマンド送信
		ParticleCommand command = MakeCommand<float>(ParticleCommandTarget::Updater,
			ParticleCommandID::Scaling, module.updaterScaleValue);
		EffectCommandRouter::Send(system, command);
	}

	// LifeEndMode の外部切替
	if (module.sendLifeEndMode) {

		// 寿命設定コマンド送信
		ParticleCommand command = MakeCommand<ParticleLifeEndMode>(ParticleCommandTarget::Updater,
			ParticleCommandID::SetLifeEndMode, module.lifeEndMode);
		EffectCommandRouter::Send(system, command);
	}
}

void EffectModuleBinder::ApplyUpdate(ParticleSystem* system, const EffectModuleSetting& module,
	const Vector3& parentTranslation, const Quaternion& parentRotation,
	[[maybe_unused]] const EffectCommandContext& context) {

	// 無効な場合は処理しない
	if (!system) {
		return;
	}

	// 回転
	if (module.sendUpdaterRotation && module.updateRotateOption == EffectUpdateRotateOption::UseUpdateEuler) {

		// 回転コマンド送信
		const Quaternion rotation = Quaternion::EulerToQuaternion(module.updateRotate);
		ParticleCommand command = MakeCommand<Quaternion>(ParticleCommandTarget::Updater,
			ParticleCommandID::SetRotation, rotation);
		// 回転モジュールのみ
		command.filter.updaterId = ParticleUpdateModuleID::Rotation;
		EffectCommandRouter::Send(system, command);
	}

	// コマンドに設定する位置
	Vector3 worldPos = parentTranslation + module.spawnPos;
	// ノード回転を適用する場合
	if (module.posOption == EffectPosOption::ApplyNodeRotation) {

		worldPos = parentRotation * module.spawnPos + parentTranslation;
	}

	// 座標固定モジュール
	if (module.sendUpdaterTranslate) {

		// 位置コマンド送信
		ParticleCommand command = MakeCommand<Vector3>(ParticleCommandTarget::Updater,
			ParticleCommandID::SetTranslation, worldPos);
		// 座標固定モジュールのみ
		command.filter.updaterId = ParticleUpdateModuleID::Translate;
		EffectCommandRouter::Send(system, command);
	}

	// KeyframePath
	if (module.sendUpdaterKeyPath) {

		// 位置コマンド送信
		ParticleCommand command = MakeCommand<Vector3>(ParticleCommandTarget::Updater,
			ParticleCommandID::SetTranslation, worldPos);
		// KeyframePathモジュールのみ
		command.filter.updaterId = ParticleUpdateModuleID::KeyframePath;
		EffectCommandRouter::Send(system, command);
	}

	// LifeEndMode
	if (module.sendLifeEndMode) {

		// 寿命設定コマンド送信
		ParticleCommand command = MakeCommand<ParticleLifeEndMode>(ParticleCommandTarget::Updater,
			ParticleCommandID::SetLifeEndMode, module.lifeEndMode);
		EffectCommandRouter::Send(system, command);
	}
}