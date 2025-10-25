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
	EffectModuleBinder::ApplyPreEmit(system, module, worldPos);

	// FrequencyEmitで常に発生
	if (emit.mode == EffectEmitMode::Always) {

		system->FrequencyEmit();
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