#include "EffectSequencer.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>

//============================================================================
//	EffectSequencer classMethods
//============================================================================

bool EffectSequencer::ConsumeOffset(EffectNodeRuntime* runtime) {

	// ランタイムが無効なら何もしない
	if (!runtime) {
		return false;
	}
	// 開始オフセットが残っていれば、スケール済みで減算
	if (0.0f < runtime->startOffsetRemain) {

		runtime->startOffsetRemain -= GameTimer::GetScaledDeltaTime();
		// 0.0f未満になったら0.0fに補正
		if (runtime->startOffsetRemain < 0.0f) {

			runtime->startOffsetRemain = 0.0f;
		}
	}
	
	//  オフセットが消化されていれば発生可能
	bool result = runtime->startOffsetRemain <= 0.0f;
	return result;
}

bool EffectSequencer::CheckStartAfter(const EffectStartAfter& after,
	const std::unordered_map<std::string, EffectNodeSignals>& signals,
	const EffectQueryGroupAliveFn& queryFn) {

	// 条件なし、参照先ノード未指定なら常にtrue
	if (after.condition == EffectSequencerStartCondition::None || after.emptyRef.nodeKey.empty()) {
		return true;
	}
	auto it = signals.find(after.emptyRef.nodeKey);
	if (it == signals.end()) {
		return false;
	}

	//  依存先ノードのシグナルを取得
	const EffectNodeSignals& signal = it->second;
	switch (after.condition) {
	case EffectSequencerStartCondition::OnStart: {

		return signal.started;
	}
	case EffectSequencerStartCondition::OnFirstEmit: {

		return signal.firstEmitted;
	}
	case EffectSequencerStartCondition::OnStop: {

		return signal.stopped;
	}
	case EffectSequencerStartCondition::OnEmpty: {

		// 参照先ノードの指定グループの生存パーティクル数が0かどうか
		if (!queryFn || after.emptyRef.groupIndex < 0) {

			return false;
		}
		return queryFn(after.emptyRef) == 0;
	}
	}
	return true;
}