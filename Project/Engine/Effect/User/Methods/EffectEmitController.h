#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/Methods/EffectStructures.h>

//============================================================================
//	EffectEmitController class
//	EmitModeに基づく発生状態の制御、発生タイミングの管理
//============================================================================
class EffectEmitController {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	EffectEmitController() = default;
	~EffectEmitController() = default;

	// ノードの外部開始
	void Start(EffectNodeRuntime* runtime);
	// 外部停止
	void Stop(EffectNodeRuntime* runtime);

	// 再生開始時の初期化
	void Reset(const EffectEmitSetting& emit, EffectNodeRuntime* runtime);

	// 現在の設定とランタイムから、発生させるかどうかを判定して返す
	bool Tick(const EffectEmitSetting& emit, EffectNodeRuntime* runtime);
};