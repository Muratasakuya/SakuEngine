#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Module/ParticleModuleID.h>
#include <Engine/MathLib/MathUtils.h>

// c++
#include <string>
#include <optional>
#include <variant>

//============================================================================
//	ParticleCommand
//============================================================================

// コマンド処理先
enum class ParticleCommandTarget {

	All,
	Spawner,
	Updater
};

// コマンドの識別ID
enum class ParticleCommandID {

	// 発生モジュール
	SetEmitFlag,          // 発生の設定
	SetBillboardRotation, // ビルボード回転の設定

	// 更新モジュール
	SetLifeEndMode,      // パーティクルの寿命管理設定
	Scaling,             // スケーリング処理
	SetKeyframePath,     // キーフレームパスの設定
	SetLightningSegment, // 雷の位置設定

	// 共通
	SetTranslation,    // 座標の設定
	SetRotation,       // 回転の設定
	SetParentRotation, // 親回転の設定
};

// 寿命が尽きた時の処理
enum class ParticleLifeEndMode {

	Advance, // 次のフェーズに進む
	Clamp,   // 最大時間で固定、次にも進まない
	Reset,   // 時間をリセットして再処理
	Kill     // 即削除
};

// 特定のIDの指定
struct ParticleCommandFilter {

	std::optional<ParticleUpdateModuleID> updaterId;
};

// 使用できる型
using ParticleCommandValue = std::variant<
	bool, int32_t, uint32_t, float, Vector3, std::vector<Vector3>,
	Vector4, Quaternion, Color, Matrix4x4, ParticleLifeEndMode>;

struct ParticleCommand {

	ParticleCommandTarget target; // コマンド転送先モジュール
	ParticleCommandID id;         // コマンドの種類

	ParticleCommandFilter filter; // 更新モジュールの指定
	ParticleCommandValue  value;  // コマンドに合わせた渡す値
};