#include "EffectNode.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/System/ParticleSystem.h>
#include <Engine/Effect/User/Methods/EffectSequencer.h>
#include <Engine/Effect/User/Methods/EffectModuleBinder.h>

//============================================================================
//	EffectNode classMethods
//============================================================================

void EffectNode::Update(const Vector3& worldPos, const EffectQueryGroupAliveFn& queryFn) {

	// ランタイムが無効なら何もしない
	if (!system) {
		return;
	}

	// Manual以外はシーケンサの開始遅延を消化、まだならこのフレームはスキップ
	if (emit.mode != EffectEmitMode::Manual) {
		// 
		if (!EffectSequencer::ConsumeOffset(&runtime)) {
			return;
		}
	}

	// 発生前のコマンドを設定
	EffectCommandContext context{};
	EffectModuleBinder::ApplyPreEmit(system, module, worldPos, context);
	// Updaterモジュールの更新コマンドを設定
	const Vector3 parentTranslation = worldPos - module.spawnPos;
	const Quaternion parentRotation = Quaternion::Identity();
	EffectModuleBinder::ApplyUpdate(system, module, parentTranslation, parentRotation, context);

	// FrequencyEmitで常に発生
	if (emit.mode == EffectEmitMode::Always) {

		// Tickで時間を更新する
		emitController.Tick(emit, &runtime);

		// アクティブ状態の時のみ時間で発生
		if (runtime.active && !runtime.pending) {

			system->FrequencyEmit();
			if (!runtime.started) {
				runtime.started = true;
			}
			if (!runtime.didFirstEmit) {
				runtime.didFirstEmit = true;
			}
		}
	}
	// Tickで判定して発生
	else if (emitController.Tick(emit, &runtime)) {

		system->Emit();
		if (!runtime.started) {

			runtime.started = true;
		}
	}

	// 停止条件を満たしたら非アクティブ状態にして発生しないようにする
	if (stopController.ShouldStop(stop, emit, runtime, queryFn)) {

		runtime.active = false;
		runtime.stopped = true;
	}
}

void EffectNode::Reset() {

	// ランタイム、発生情報をリセット
	runtime = EffectNodeRuntime{};
	runtime.startOffsetRemain = sequencer.startOffset;
	emitController.Reset(emit, &runtime);
}