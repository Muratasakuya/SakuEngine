#include "BossEnemyGreatAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

// state
#include <Game/Objects/GameScene/Enemy/Boss/State/States/GreatAttackState/States/BossEnemyGreatAttackBlowPlayer.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/GreatAttackState/States/BossEnemyGreatAttackCharge.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/GreatAttackState/States/BossEnemyGreatAttackExecute.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/GreatAttackState/States/BossEnemyGreatAttackFinish.h>

//============================================================================
//	BossEnemyGreatAttackState classMethods
//============================================================================

BossEnemyGreatAttackState::BossEnemyGreatAttackState() {

	// 各状態を初期化
	states_.emplace(State::BlowPlayer, std::make_unique<BossEnemyGreatAttackBlowPlayer>());
	states_.emplace(State::Charge, std::make_unique<BossEnemyGreatAttackCharge>());
	states_.emplace(State::Execute, std::make_unique<BossEnemyGreatAttackExecute>());
	states_.emplace(State::Finish, std::make_unique<BossEnemyGreatAttackFinish>());

	// 初期化値
	canExit_ = false;
	editState_ = State::BlowPlayer;
}

void BossEnemyGreatAttackState::InitState(BossEnemy& bossEnemy) {

	for (const auto& state : std::views::values(states_)) {

		state->Init(&bossEnemy, player_, followCamera_);
	}
}

void BossEnemyGreatAttackState::Enter([[maybe_unused]] BossEnemy& bossEnemy) {

	// 初期状態で初期化
	currentState_ = State::BlowPlayer;
	states_[currentState_]->Enter();
}

void BossEnemyGreatAttackState::Update([[maybe_unused]] BossEnemy& bossEnemy) {

	// 現在の状態を更新
	auto& state = states_[currentState_];
	state->Update();

	// 処理終了後次の状態に進む
	if (state->CanExit()) {

		// 現在の状態を終了させる
		state->Exit();
		// 次の状態があれば遷移
		if (auto next = GetNextState(currentState_)) {

			currentState_ = *next;
			states_[currentState_]->Enter();
		} else {

			// 処理終了
			canExit_ = true;
		}
	}
}

void BossEnemyGreatAttackState::Exit([[maybe_unused]] BossEnemy& bossEnemy) {

	canExit_ = false;

	// 全てのExitを呼びだして完全にリセット
	for (const auto& state : std::views::values(states_)) {

		state->Exit();
	}
}

void BossEnemyGreatAttackState::ImGui([[maybe_unused]] const BossEnemy& bossEnemy) {

	ImGui::Text(std::format("canExit: {}", canExit_).c_str());
	ImGui::Text("currentState: %s", EnumAdapter<State>::ToString(currentState_));
	EnumAdapter<State>::Combo("State", &editState_);

	ImGui::Separator();
	states_[editState_]->ImGui();
}

void BossEnemyGreatAttackState::ApplyJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	for (const auto& [state, ptr] : states_) {

		auto key = EnumAdapter<State>::ToString(state);
		ptr->ApplyJson(data[key]);
	}
}

void BossEnemyGreatAttackState::SaveJson(Json& data) {

	for (const auto& [state, ptr] : states_) {

		auto key = EnumAdapter<State>::ToString(state);
		ptr->SaveJson(data[key]);
	}
}

std::optional<BossEnemyGreatAttackState::State>
BossEnemyGreatAttackState::GetNextState(State state) const {

	// 次の遷移状態を返す、無ければnullopt
	switch (state) {
	case State::BlowPlayer: return State::Charge;
	case State::Charge:     return State::Execute;
	case State::Execute:    return State::Finish;
	case State::Finish:     return std::nullopt;
	}
	return std::nullopt;
}