#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Player/State/Interface/PlayerIState.h>
#include <Game/Objects/GameScene/Player/Structures/PlayerStructures.h>

// c++
#include <memory>
#include <optional>
#include <unordered_map>
// imgui
#include <imgui.h>

//============================================================================
//	PlayerStateController class
//	プレイヤーの状態管理
//============================================================================
class PlayerStateController {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerStateController() = default;
	~PlayerStateController() = default;

	void Init(Player& owner);

	void Update(Player& owner);

	void ImGui(const Player& owner);

	//--------- accessor -----------------------------------------------------

	void SetBossEnemy(const BossEnemy* bossEnemy);
	void SetFollowCamera(FollowCamera* followCamera);

	void SetStatas(const PlayerStats& stats) { stats_ = stats; }
	void SetForcedState(Player& owner, PlayerState state);

	PlayerState GetCurrentState() const { return current_; }
	PlayerState GetSwitchSelectState() const;

	bool IsTriggerParry() const { return inputMapper_->IsTriggered(PlayerInputAction::Parry); }
	bool IsActiveParry() const { return parrySession_.active; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// パリィ処理
	struct ParrySession {

		bool active = false;    // 処理中か
		bool reserved = false;  // タイミング待ち
		uint32_t total = 0; // 連続回数
		uint32_t done = 0;  // 処理済み回数
		float reservedStart = 0.0f;

		void Init();
	};

	//--------- variables ----------------------------------------------------

	const BossEnemy* bossEnemy_;

	const std::string kStateJsonPath_ = "Player/stateParameter.json";

	// 入力
	std::unique_ptr<InputMapper<PlayerInputAction>> inputMapper_;
	// ステータス
	PlayerStats stats_;
	// パリィ処理
	ParrySession parrySession_;

	std::unordered_map<PlayerState, std::unique_ptr<PlayerIState>> states_;

	// 各状態の遷移条件
	std::unordered_map<PlayerState, PlayerStateCondition> conditions_;

	// 各状態の遷移クールタイム
	std::unordered_map<PlayerState, float> lastEnterTime_;
	float currentEnterTime_;

	// 受付済みコンボ
	std::optional<PlayerState> queued_;

	PlayerState current_;                  // 現在の状態
	std::optional<PlayerState> requested_; // 次の状態

	// editor
	int editingStateIndex_;
	int comboIndex_;
	// エディターと同期中の状態
	std::optional<PlayerState> externalSynchState_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// update
	void UpdateInputState();
	void UpdateParryState(Player& owner);
	void RequestParryState();
	bool UpdateExternalSynch(Player& owner);

	// helper
	void SetInputMapper();
	bool Request(PlayerState state);
	void ChangeState(Player& owner);
	void HandleStunTransition(Player& owner);
	bool CanTransition(PlayerState next, bool viaQueue) const;
	bool IsCombatState(PlayerState state) const;
	bool IsInChain() const;
	bool IsStunProcessing() const;
};