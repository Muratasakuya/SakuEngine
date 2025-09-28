#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/MathUtils.h>

// c++
#include <cstdint>
#include <vector>

//============================================================================
//	BossEnemyStructures class
//============================================================================

// memo
// stateTableを作成する、これを基にstateを変更できるようにする
// 各phaseには共通のパラメータとして次のstateへ遷移するまでの時間を設定できるようにする
// []のなかに入る組み合わせをComboとし、作成できるようにする
// Comboの流れをvectorに格納し[]内に入れられるようにする(1つでもComboとする)
// PhaseでAddStateをすると[]を追加できる、とりあえずデフォルトでIdle(同じStateは選択できない)
// []のなかでドロップダウンでComboを選択できるようにする
// 各Comboの中のパラメータとしてstateが切り替わったときに、
// また同じstate(Combo処理)になってもいいのかだめなのか設定できるようにする
// Phase0 [Idle][LightAttack]
// Phase1 [Idle][LightAttack][Teleportation -> ChargeAttack]
// Phase2 [Idle][LightAttack][StrongAttack][RushAttack]

// 状態の種類
enum class BossEnemyState {

	Idle,            // 何もしない
	Teleport,        // 瞬間移動(実際には高速で補間する)
	Stun,            // スタン状態
	Falter,          // 怯む
	LightAttack,     // 弱攻撃
	StrongAttack,    // 強攻撃
	ChargeAttack,    // 溜め攻撃
	RushAttack,      // 突進攻撃
	ContinuousAttack // 連続攻撃
};

// テレポートの種類
enum class BossEnemyTeleportType {

	Far, // 遠くに
	Near // 近くに
};

// ステータス
struct BossEnemyStats {

	int maxHP;     // 最大HP
	int currentHP; // 現在のHP

	// 閾値リストの条件
	// indexNはindexN+1の値より必ず大きい(N=80、N+1=85にはならない)
	std::vector<int> hpThresholds; // HP割合の閾値リスト

	std::unordered_map<BossEnemyState, int> damages; // 各攻撃のダメージ量
	int damageRandomRange;                           // ダメージのランダム範囲

	int maxDestroyToughness;     // 撃破靭性値
	int currentDestroyToughness; // 現在の撃破靭性値
};

// コンボリスト
struct BossEnemyCombo {

	std::vector<BossEnemyState> sequence; // コンボの順序
	bool allowRepeat;                     // 同じComboを繰り返してもよいか
	BossEnemyTeleportType teleportType;   // テレポートの種類

	void FromJson(const Json& data);
	void ToJson(Json& data);
};

// 各フェーズのパラメータ
struct BossEnemyPhase {

	float nextStateDuration = 1.0f; // この秒数経過で次状態へ遷移
	std::vector<int> comboIndices;  // コンボインデックスのリスト
	bool autoIdleAfterAttack; // 強制的に待機状態に戻すか

	void FromJson(const Json& data);
	void ToJson(Json& data);
};

// ボスの状態テーブル
struct BossEnemyStateTable {

	std::vector<BossEnemyCombo> combos;
	std::vector<BossEnemyPhase> phases;

	void FromJson(const Json& data);
	void ToJson(Json& data);
};

// パリィデータ
// 連続パリィが可能な場合、すべて受けきった後に攻撃可能
struct ParryParameter {

	bool canParry = false;        // パリィ可能かどうか
	uint32_t continuousCount = 0; // パリィ回数(連続パリィ)
};