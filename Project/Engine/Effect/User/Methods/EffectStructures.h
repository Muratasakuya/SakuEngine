#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector3.h>
#include <Engine/Effect/Particle/Command/ParticleCommand.h>

// c++
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

//============================================================================
//	EffectStructures
//	ゲームで使用するエフェクト関連の構造体定義
//============================================================================

// 発生の仕方
enum class EffectEmitMode {

	Once,      // 1回だけ発生
	Always,    // 常に発生させる
	EmitCount, // インターバルで指定回数発生
	Manual     // 外部からの入力で停止させる
};

// 停止条件
enum class EffectStopCondition {

	None,
	AfterDuration,   // 時間経過で
	OnParticleEmpty, // 対象のグループのパーティクルがすべてなくなったら
	ExternalStop     // 外部入力
};

// 発生姿勢のプリセット
enum class EffectPosePreset {

	None,
	BillboardToCamera,
};

// シーケンサーの開始条件
enum class EffectSequencerStartCondition {

	None,
	OnStart,
	OnFirstEmit,
	OnStop,
	OnEmpty,
};

// 依存の参照先
struct EffectDependencyReference {

	std::string nodeKey; // 参照先ノード
	int groupIndex = -1; // 参照先ノードのCPUグループインデックス
};

// 参照先ノードのCPUグループのパーティクル生存数を取得する関数型
using EffectQueryGroupAliveFn = std::function<uint32_t(EffectDependencyReference ref)>;

// 発生設定
struct EffectEmitSetting {

	EffectEmitMode mode = EffectEmitMode::Once;
	int count = 1;         // modeがEmitCountのときの発生回数
	float duration = 0.0f; // modeがAfterDurationの時の発生間隔
	float delay = 0.0f;    // Emit呼出しからの遅延
	float interval = 0.0f; // 連続発生する際の間隔
};

// 停止設定
struct EffectStopSetting {

	EffectStopCondition condition = EffectStopCondition::None;
	EffectDependencyReference emptyRef; // conditionがOnParticleEmpty用の参照
};

// モジュール設定
struct EffectModuleSetting {

	// 発生座標、回転
	Vector3 spawnPos = Vector3(0.0f, 0.4f, 0.0f);
	Vector3 spawnRotate = Vector3::AnyInit(0.0f);
	EffectPosePreset posePreset = EffectPosePreset::None;

	// 発生モジュールの設定
	bool spawnerScaleEnable = false;
	float spawnerScaleValue = 1.0f;

	// 更新モジュールの設定
	bool updaterScaleEnable = false;
	float updaterScaleValue = 1.0f;
	ParticleLifeEndMode lifeEndMode = ParticleLifeEndMode::Advance;
};

// どの条件で開始するかの依存先
struct EffectStartAfter {

	EffectSequencerStartCondition condition = EffectSequencerStartCondition::None;
	EffectDependencyReference emptyRef; // conditionがOnEmpty用の参照
};

// シーケンサー設定
struct EffectSequencerSetting {

	float startOffset = 0.0f;
	EffectStartAfter startAfter;
};

// ランタイム処理中の情報
struct EffectNodeRuntime {

	bool pending = false;      // delay待ち中か
	bool active = false;       // 有効かどうか
	bool started = false;      // 発生処理を開始したかどうか
	bool stopped = false;      // 停止フラグ
	bool didFirstEmit = false; // 1回目の発生を行ったかどうか
	int emitted = 0;           // 発生済み回数
	float timer = 0.0f;        // 発生してから(Emitを呼びだしてから)の経過時間
	float emitTimer = 0.0f;    // 発生間隔管理時間
	float startOffsetRemain = 0.0f; // 発生待ち中の時間
};