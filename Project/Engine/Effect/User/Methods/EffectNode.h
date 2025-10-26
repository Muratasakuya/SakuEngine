#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/Methods/EffectEmitController.h>
#include <Engine/Effect/User/Methods/EffectStopController.h>

// front
class ParticleSystem;

//============================================================================
//	EffectNode class
//	ParticleSysteとランタイムを保持し、前処理、発生、停止制御を行う
//============================================================================
struct EffectNode {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	EffectNode() = default;
	~EffectNode() = default;

	//--------- variables ----------------------------------------------------

	// パーティクルシステム
	ParticleSystem* system = nullptr;

	std::string name;     // 表示名
	std::string key;      // 参照用
	std::string filePath; // 読み込み元ファイルパス

	EffectEmitSetting emit;           // 発生設定
	EffectStopSetting stop;           // 停止設定
	EffectModuleSetting module;       // 発生前コマンドの設定
	EffectSequencerSetting sequencer; // 開始遅延と依存トリガーの設定

	// ランタイム情報
	EffectNodeRuntime runtime;

	EffectEmitController emitController; // 発生制御
	EffectStopController stopController; // 停止制御

	//--------- functions ----------------------------------------------------
	
	// 前処理、発生、停止判定を行う
	void Update(const Vector3& worldPos, const EffectQueryGroupAliveFn& queryFn);

	// リセット
	void Reset();
};