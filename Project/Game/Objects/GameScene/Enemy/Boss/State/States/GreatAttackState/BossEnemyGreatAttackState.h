#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Enemy/Boss/State/Interface/BossEnemyIState.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/GreatAttackState/Interface/BossEnemyGreatAttackIState.h>

//============================================================================
//	BossEnemyGreatAttackState class
//	ボスの大技攻撃処理
//============================================================================
class BossEnemyGreatAttackState :
	public BossEnemyIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyGreatAttackState();
	~BossEnemyGreatAttackState() = default;

	void InitState(BossEnemy& bossEnemy);

	void Enter(BossEnemy& bossEnemy) override;

	void Update(BossEnemy& bossEnemy) override;

	void Exit(BossEnemy& bossEnemy) override;

	// imgui
	void ImGui(const BossEnemy& bossEnemy) override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		BlowPlayer, // プレイヤー吹っ飛ばし
		Charge,     // チャージ開始 -> 終了後魔法陣起動
		Execute,    // 大技攻撃中...(魔法陣回転中)
		Finish      // 終了(魔法陣を閉じる)
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// 状態マップ
	std::unordered_map<State, std::unique_ptr<BossEnemyGreatAttackIState>> states_;

	// 値操作する状態
	State editState_;

	//--------- functions ----------------------------------------------------

	// helper
	std::optional<State> GetNextState(State state) const;
};