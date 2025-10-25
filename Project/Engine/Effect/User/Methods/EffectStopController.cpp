#include "EffectStopController.h"

//============================================================================
//	EffectStopController classMethods
//============================================================================

bool EffectStopController::ShouldStop(const EffectStopSetting& stop, const EffectEmitSetting& emit,
	const EffectNodeRuntime& runtime, const EffectQueryGroupAliveFn& queryFn) const {

	switch (stop.condition) {
	case EffectStopCondition::AfterDuration: {

		// 指定時間経過していたら停止させる
		bool result = emit.duration <= runtime.timer;
		return result;
	}
	case EffectStopCondition::OnParticleEmpty: {

		if (!stop.emptyRef.nodeKey.empty() && 0 <= stop.emptyRef.groupIndex && queryFn) {

			// 参照先ノードの指定グループの生存パーティクル数が0なら停止させる
			// GetNumInstance() == 0
			return queryFn(stop.emptyRef) == 0;
		}
		return false;
	}
	case EffectStopCondition::ExternalStop: {

		// 外部入力による停止はここでは判定しない
		return false;
	}
	}
	return false;
}