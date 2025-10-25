#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/Methods/EffectStructures.h>

// c++
#include <unordered_map>

//============================================================================
//	EffectSequencer structure
//============================================================================

// 
struct EffectNodeSignals {

	bool started = false;      // ノードが一度でも起動したか
	bool firstEmitted = false; // 最初の発生処理が行われたか
	bool stopped = false;      // 停止処理が行われたか
};

//============================================================================
//	EffectSequencer class
//	開始遅延、依存トリガーの判定
//============================================================================
class EffectSequencer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	EffectSequencer() = default;
	~EffectSequencer() = default;

	// startOffsetを経過時間で消化し、発生可能かどうか返す
	static bool ConsumeOffset(EffectNodeRuntime* runtime);

	// 依存先の条件を満たしているかチェックして返す
	static bool CheckStartAfter(const EffectStartAfter& after,
		const std::unordered_map<std::string, EffectNodeSignals>& signals,
		const EffectQueryGroupAliveFn& queryFn);
};