#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Enemy/Boss/Structures/BossEnemyStructures.h>
#include <Game/Objects/GameScene/Player/Structures/PlayerStructures.h>

// front
class BossEnemy;
class Player;

//============================================================================
//	BossEnemyRequestFalter class
//	ボスを怯ませるかチェックしてフラグを返す
//============================================================================
class BossEnemyRequestFalter {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyRequestFalter() = default;
	~BossEnemyRequestFalter() = default;

	// 初期化
	void Init(const BossEnemy* bossEnemy, const Player* player);

	// エディター
	void ImGui();

	// 怯むかチェックして返す
	bool Check();

	//--------- accessor -----------------------------------------------------

	// 攻撃回数カウントを増やす
	void IncrementRecoverCount();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	// ボスの状態リスト
	struct BossEnemyStateInfo {

		bool isAllow = false; // 怯みを許可するか
	};

	// プレイヤーの状態リスト
	struct PlayerStateInfo {

		bool isForce = false; // 強制的に怯むか
	};

	//--------- variables ----------------------------------------------------

	// 参照
	const BossEnemy* bossEnemy_ = nullptr;
	const Player* player_ = nullptr;

	// 怯みを許可するボスの状態リスト
	std::unordered_map<BossEnemyState, BossEnemyStateInfo> allowFalterBossInfos_;
	// 攻撃を受けたら怯むプレイヤーの状態リスト
	std::unordered_map<PlayerState, PlayerStateInfo> falterPlayerInfos_;

	// 怯み回数カウント
	uint32_t currentFalterCount_;
	// 怯み不可にするまでの回数
	uint32_t maxFalterCount_;

	// 怯まなくなってからの攻撃回数カウント
	uint32_t currentRecoverFalterCount_;
	// 怯まなくなった後、怯めるようにするまでの攻撃回数
	uint32_t recoverFalterCount_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};