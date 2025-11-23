#include "PlayerStateController.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

// inputDevice
#include <Game/Objects/GameScene/Player/Input/Device/PlayerKeyInput.h>
#include <Game/Objects/GameScene/Player/Input/Device/PlayerGamePadInput.h>

// state
#include <Game/Objects/GameScene/Player/State/States/PlayerIdleState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerWalkState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerDashState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerAvoidSatate.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerAttack_1stState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerAttack_2ndState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerAttack_3rdState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerAttack_4thState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerSkilAttackState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerParryState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerSwitchAllyState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerStunAttackState.h>
#include <Game/Objects/GameScene/Player/State/States/PlayerFalterState.h>

//============================================================================
//	PlayerStateController classMethods
//============================================================================

void PlayerStateController::Init(Player& owner) {

	// 入力クラスを初期化
	Input* input = Input::GetInstance();
	inputMapper_ = std::make_unique<InputMapper<PlayerInputAction>>();
	inputMapper_->AddDevice(std::make_unique<PlayerGamePadInput>(input));

#ifdef _RELEASE
	//inputMapper_->AddDevice(std::make_unique<PlayerKeyInput>(input));
#endif

	// 各状態を初期化
	states_.emplace(PlayerState::Idle, std::make_unique<PlayerIdleState>());
	states_.emplace(PlayerState::Walk, std::make_unique<PlayerWalkState>());
	states_.emplace(PlayerState::Dash, std::make_unique<PlayerDashState>());
	states_.emplace(PlayerState::Avoid, std::make_unique<PlayerAvoidSatate>(&owner));
	states_.emplace(PlayerState::Attack_1st, std::make_unique<PlayerAttack_1stState>(&owner));
	states_.emplace(PlayerState::Attack_2nd, std::make_unique<PlayerAttack_2ndState>(&owner));
	states_.emplace(PlayerState::Attack_3rd, std::make_unique<PlayerAttack_3rdState>(&owner));
	states_.emplace(PlayerState::Attack_4th, std::make_unique<PlayerAttack_4thState>(&owner));
	states_.emplace(PlayerState::SkilAttack, std::make_unique<PlayerSkilAttackState>(&owner));
	states_.emplace(PlayerState::Parry, std::make_unique<PlayerParryState>());
	states_.emplace(PlayerState::SwitchAlly, std::make_unique<PlayerSwitchAllyState>());
	states_.emplace(PlayerState::StunAttack, std::make_unique<PlayerStunAttackState>(owner.GetAlly()));
	states_.emplace(PlayerState::Falter, std::make_unique<PlayerFalterState>(&owner));

	// json適応
	ApplyJson();

	// inputを設定
	SetInputMapper();

	// 初期状態を設定
	current_ = PlayerState::Idle;
	requested_ = PlayerState::Idle;
	currentEnterTime_ = GameTimer::GetTotalTime();
	lastEnterTime_[current_] = currentEnterTime_;
	isDashInput_ = false;
	ChangeState(owner);
}

void PlayerStateController::SetInputMapper() {

	// 各状態にinputをセット
	for (const auto& state : std::views::values(states_)) {

		state->SetInputMapper(inputMapper_.get());
	}
}

void PlayerStateController::SetBossEnemy(const BossEnemy* bossEnemy) {

	// 各状態にbossEnemyをセット
	for (const auto& state : std::views::values(states_)) {

		state->SetBossEnemy(bossEnemy);
	}
	bossEnemy_ = nullptr;
	bossEnemy_ = bossEnemy;
}

void PlayerStateController::SetFollowCamera(FollowCamera* followCamera) {

	// 各状態にfollowCameraをセット
	for (const auto& state : std::views::values(states_)) {

		state->SetFollowCamera(followCamera);
	}
}

void PlayerStateController::SetForcedState(Player& owner, PlayerState state) {

	// 同じ状態への強制遷移が許可されていれば
	if (current_ == state && conditions_.at(state).enableInARowForceState) {

		// 状態をリセットして再度状態を処理する
		if (const auto& currentState = states_[current_].get()) {

			currentState->Exit(owner);
			currentState->Enter(owner);
		}
		currentEnterTime_ = GameTimer::GetTotalTime();
		lastEnterTime_[current_] = currentEnterTime_;
		owner.GetAttackCollision()->SetEnterState(current_);
		return;
	}

	// 全ての行動を削除
	queued_.reset();
	requested_.reset();

	// 現在の行動を強制終了
	if (auto* currentState = states_[current_].get()) {

		currentState->Exit(owner);
	}

	// 次の状態を設定
	PlayerState preState = current_;
	current_ = state;

	// 遷移入り
	if (auto* currentState = states_[current_].get()) {

		currentState->SetPreState(preState);
		currentState->Enter(owner);
	}

	// 現在の時間を記録
	currentEnterTime_ = GameTimer::GetTotalTime();
	lastEnterTime_[current_] = currentEnterTime_;
	owner.GetAttackCollision()->SetEnterState(current_);
}

void PlayerStateController::RequestFalterState(Player& owner) {

	// 現在の状態が攻撃を受けたら怯む状態なら遷移させる
	if (conditions_.at(current_).isArmor) {
		// 怯み無効状態なら遷移させない
		return;
	}

	// 怯み状態に遷移させる
	SetForcedState(owner, PlayerState::Falter);
}

PlayerState PlayerStateController::GetSwitchSelectState() const {

	// SwitchAlly状態の時になにをplayerが選択したのか取得する
	return static_cast<PlayerSwitchAllyState*>(states_.at(PlayerState::SwitchAlly).get())->GetSelectState();
}

bool PlayerStateController::IsAvoidance() const {

	// 現在の状態で回避行動を行っているかどうか
	if (auto* currentState = states_.at(current_).get()) {

		return currentState->IsAvoidance();
	}
	// それ以外はfalse
	return false;
}

void PlayerStateController::Update(Player& owner) {

	// 外部進捗による更新中なら入力による状態遷移を行わない
	if (UpdateExternalSynch(owner)) {
		return;
	}

	// 入力に応じた状態の遷移
	UpdateInputState(owner);

	// パリィ処理
	UpdateParryState(owner);

	// 何か予約設定されて入れば状態遷移させる
	if (queued_) {
		bool canInterrupt = false;
		if (auto it = conditions_.find(*queued_); it != conditions_.end()) {

			canInterrupt = it->second.CheckInterruptableByState(current_);
		}

		if ((states_[current_]->GetCanExit() || canInterrupt) &&
			CanTransition(*queued_, !canInterrupt)) {

			requested_ = queued_;
			queued_.reset();
		}
	}

	// 何か設定されていれば遷移させる
	if (requested_.has_value()) {

		ChangeState(owner);
	}

	// パリィの状態管理
	RequestParryState();

	// 敵がスタン中の状態遷移処理
	HandleStunTransition(owner);

	// 常に更新する値
	for (const auto& [state, ptr] : states_) {

		if (state == PlayerState::None) {
			continue;
		}
		ptr->BeginUpdateAlways(owner);
	}

	// 現在の状態を更新
	if (PlayerIState* currentState = states_[current_].get()) {

		currentState->Update(owner);
	}

	// 常に更新する値
	for (const auto& [state, ptr] : states_) {

		if (state == PlayerState::None) {
			continue;
		}
		ptr->UpdateAlways(owner);
	}

	// Y座標の制限
	owner.ClampInitPosY();
}

void PlayerStateController::UpdateInputState(Player& owner) {

	// スタン処理中は状態遷移不可
	if (IsStunProcessing()) {
		return;
	}

	// コンボ中は判定をスキップする
	bool inCombat = IsCombatState(current_);
	bool actionLocked = (inCombat && !states_.at(current_)->GetCanExit()) || (inCombat && IsInChain());

	// 移動方向
	Vector2 move(inputMapper_->GetVector(PlayerInputAction::MoveX),
		inputMapper_->GetVector(PlayerInputAction::MoveZ));
	// 動いたかどうか判定
	bool isMove = move.Length() > std::numeric_limits<float>::epsilon();

	// 歩き、待機状態の状態遷移
	{
		if (!actionLocked && current_ != PlayerState::Dash) {

			// 移動していた場合は歩き、していなければ待機状態のまま
			if (isMove) {

				Request(PlayerState::Walk);
			} else {

				Request(PlayerState::Idle);
			}
		}
	}

	// ダッシュ、攻撃の状態遷移
	{

		// ダッシュ入力があったかどうか
		if (inputMapper_->IsTriggered(PlayerInputAction::Dash)) {

			isDashInput_ = true;
		}
		// 移動していなければダッシュ入力をリセット
		if (!isMove) {

			isDashInput_ = false;
		}

		// 移動している時にダッシュ入力があればダッシュ状態に遷移
		if (isMove && isDashInput_) {

			Request(PlayerState::Dash);
		} else if (!isMove && current_ == PlayerState::Dash) {

			// 移動が止まったらダッシュ終了
			Request(PlayerState::Idle);
		}

		if (inputMapper_->IsTriggered(PlayerInputAction::Attack)) {

			if (current_ == PlayerState::Attack_1st) {

				Request(PlayerState::Attack_2nd);
			}
			// 2段 -> 3段
			else if (current_ == PlayerState::Attack_2nd) {
				Request(PlayerState::Attack_3rd);
			}
			// 3段 -> 4段
			else if (current_ == PlayerState::Attack_3rd) {
				Request(PlayerState::Attack_4th);
			}
			// 1段目
			else {

				// 1段目の攻撃
				Request(PlayerState::Attack_1st);
			}
			// ダッシュ入力をリセット
			isDashInput_ = false;
			return;
		}

		// スキル攻撃
		if (inputMapper_->IsTriggered(PlayerInputAction::Skill)) {

			Request(PlayerState::SkilAttack);
			return;
		}
	}

	// 回避入力
	if (!isDashInput_ && inputMapper_->IsTriggered(PlayerInputAction::Avoid)) {

		Request(PlayerState::Avoid);
		return;
	}

	// パリィの入力判定、攻撃を受けた、受けているときは無効
	if (current_ != PlayerState::Falter &&
		inputMapper_->IsTriggered(PlayerInputAction::Parry)) {

		const ParryParameter& parryParam = bossEnemy_->GetParryParam();
		if (parryParam.canParry) {

			// 入力があればパリィ処理を予約する
			parrySession_ = {};
			parrySession_.done = 0;
			parrySession_.active = true;
			parrySession_.reserved = true;
			parrySession_.total = std::max<uint32_t>(1, parryParam.continuousCount);
			parrySession_.reservedStart = GameTimer::GetTotalTime();
		}
	}

	// ダッシュ中にダッシュ入力があればダッシュ状態を再度強制遷移させる
	if (current_ == PlayerState::Dash && inputMapper_->IsTriggered(PlayerInputAction::Dash)) {

		SetForcedState(owner, PlayerState::Dash);
	}
}

void PlayerStateController::UpdateParryState(Player& owner) {

	// 敵がパリィ可能かどうかチェック
	if (parrySession_.active && parrySession_.reserved &&
		bossEnemy_ && const_cast<BossEnemy*>(bossEnemy_)->ConsumeParryTiming()) {

		++parrySession_.done;
		const bool isLast = (parrySession_.done >= parrySession_.total);

		// パリィ状態に強制遷移させる
		SetForcedState(owner, PlayerState::Parry);
		if (const auto& parryState = static_cast<PlayerParryState*>(states_.at(PlayerState::Parry).get())) {

			parryState->SetAllowAttack(isLast);
		}

		parrySession_.reserved = !isLast;
		parrySession_.reservedStart = GameTimer::GetTotalTime();
		return;
	}

	// パリィ受付をしていなければ初期化
	if (parrySession_.active &&
		bossEnemy_ && !bossEnemy_->GetParryParam().canParry) {

		parrySession_.Init();
	}
}

void PlayerStateController::RequestParryState() {

	if (current_ == PlayerState::Parry && states_[current_]->GetCanExit()) {
		if (parrySession_.done < parrySession_.total) {

			parrySession_.reservedStart = GameTimer::GetTotalTime();
		} else {

			parrySession_.Init();
		}
	}
}

bool PlayerStateController::UpdateExternalSynch(Player& owner) {

	// オブジェクトの更新が外部による更新なら
	if (owner.GetUpdateMode() == ObjectUpdateMode::External) {

		// 同期中の状態をチェック
		std::optional<PlayerState> currentActive{};
		PlayerIState* currentPtr = nullptr;
		for (auto& [state, ptr] : states_) {
			// 外部のエディターと同期中ならポインタを設定
			if (auto* base = dynamic_cast<PlayerBaseAttackState*>(ptr.get())) {
				if (base->IsExternalActive()) {

					currentActive = state;
					currentPtr = ptr.get();

					// 最初に見つかったポインタのみ
					break;
				}
			}
		}
		// 同期中の状態がなければ
		if (!currentActive.has_value()) {
			if (externalSynchState_.has_value()) {
				// Exitを呼びだしてリセットして終了
				if (PlayerIState* preState = states_[*externalSynchState_].get()) {

					preState->Exit(owner);

				}
				externalSynchState_.reset();
			}
			return true;
		}
		// 同期対象が変更されたら
		if (!externalSynchState_.has_value() || *externalSynchState_ != *currentActive) {
			// 1度Exitを呼びだしてリセットする
			if (externalSynchState_.has_value()) {
				if (PlayerIState* preState = states_[*externalSynchState_].get()) {

					preState->Exit(owner);
				}
			}
			// 同期を開始させる
			currentPtr->Enter(owner);
			externalSynchState_ = *currentActive;
		}

		// 同期中の状態を更新する
		if (currentPtr) {

			currentPtr->Update(owner);
			currentPtr->UpdateAlways(owner);
		}
		return true;
	}

	// 外部同期を終了したらExitを呼びだしてリセットして終了させる
	if (externalSynchState_.has_value()) {
		if (PlayerIState* preState = states_[*externalSynchState_].get()) {

			preState->Exit(owner);
		}
		externalSynchState_.reset();
	}
	return false;
}

bool PlayerStateController::Request(PlayerState state) {

	// 現在の状態と同じなら何もしない
	if (state == current_) {
		return false;
	}
	if (queued_ && *queued_ == state) {
		return true;
	}

	// 遷移できるかどうか判定
	const bool result = CanTransition(state, false);
	if (!result) {
		return false;
	}

	// 強制遷移可能先かチェックする
	bool canInterrupt = false;
	if (auto it = conditions_.find(state); it != conditions_.end()) {

		canInterrupt = it->second.CheckInterruptableByState(current_);
	}

	// 遷移可能か、現在の状態が終了可能かチェック
	if (!states_.at(current_)->GetCanExit() && !canInterrupt) {

		queued_ = state;
	} else {

		requested_ = state;
	}
	return true;
}

void PlayerStateController::ChangeState(Player& owner) {

	// 同じなら遷移させない
	if (requested_.value() == current_) {
		requested_ = std::nullopt;
		return;
	}

	// 現在の状態の終了処理
	if (auto* currentState = states_[current_].get()) {

		currentState->Exit(owner);
	}

	// 次の状態を設定する
	PlayerState preState = current_;
	current_ = requested_.value();

	// 次の状態を初期化する
	if (auto* currentState = states_[current_].get()) {

		currentState->SetPreState(preState);
		currentState->Enter(owner);
	}

	currentEnterTime_ = GameTimer::GetTotalTime();
	lastEnterTime_[current_] = currentEnterTime_;

	// player側に切り替えを通知する
	owner.GetAttackCollision()->SetEnterState(current_);
}

void PlayerStateController::HandleStunTransition(Player& owner) {

	// 切り替え処理
	if (current_ == PlayerState::SwitchAlly &&
		states_[current_]->GetCanExit()) {

		// 選択した状態へ遷移させる
		SetForcedState(owner, GetSwitchSelectState());
		return;
	}

	// スタン攻撃の終了判定
	if (current_ == PlayerState::StunAttack &&
		states_[current_]->GetCanExit()) {

		// 攻撃が終わったらアイドル状態に戻す
		SetForcedState(owner, PlayerState::Idle);
		return;
	}
}

bool PlayerStateController::CanTransition(PlayerState next, bool viaQueue) const {

	const auto it = conditions_.find(next);
	if (it == conditions_.end()) {
		return true;
	}

	const PlayerStateCondition& condition = it->second;
	const float totalTime = GameTimer::GetTotalTime();

	// クールタイムの処理
	auto itTime = lastEnterTime_.find(next);
	// クールタイムが終わっていなければ遷移不可
	if (itTime != lastEnterTime_.end() && totalTime - itTime->second < condition.coolTime) {

		return false;
	}

	// 強制キャンセルを行えるか判定
	if (!viaQueue) {
		if (!condition.interruptableBy.empty()) {
			const bool cancel = std::ranges::find(
				condition.interruptableBy, current_) != condition.interruptableBy.end();
			if (!cancel) {

				return false;
			}
		}
	}

	// 遷移可能な前状態かチェック
	if (!condition.allowedPreState.empty()) {
		const bool ok = std::ranges::find(condition.allowedPreState, current_) !=
			condition.allowedPreState.end();
		if (!ok) {

			return false;
		}
	}

	// コンボ入力判定
	if (!viaQueue && condition.chainInputTime > 0.0f) {
		if (totalTime - currentEnterTime_ > condition.chainInputTime) {
			return false;
		}
	}
	return true;
}

bool PlayerStateController::IsCombatState(PlayerState state) const {

	switch (state) {
	case PlayerState::Attack_1st:
	case PlayerState::Attack_2nd:
	case PlayerState::Attack_3rd:
	case PlayerState::Attack_4th:
	case PlayerState::SkilAttack:
	case PlayerState::Parry:
		return true;
	default:
		return false;
	}
}

bool PlayerStateController::IsInChain() const {

	auto it = conditions_.find(current_);
	if (it == conditions_.end()) {
		return false;
	}

	const float elapsed = GameTimer::GetTotalTime() - currentEnterTime_;
	return (it->second.chainInputTime > 0.0f) && (elapsed <= it->second.chainInputTime);
}

bool PlayerStateController::IsStunProcessing() const {

	return current_ == PlayerState::SwitchAlly ||
		current_ == PlayerState::StunAttack;
}

void PlayerStateController::ParrySession::Init() {

	active = false;    // 処理中か
	reserved = false;  // タイミング待ち
	total = 0; // 連続回数
	done = 0;  // 処理済み回数
	reservedStart = 0.0f;
}

void PlayerStateController::ImGui(const Player& owner) {

	// tool
	ImGui::Text("Current : %s", EnumAdapter<PlayerState>::ToString(current_));
	ImGui::SameLine();
	if (ImGui::Button("Save##StateJson")) {
		SaveJson();
	}

	// main
	if (ImGui::BeginTabBar("PStateTabs")) {

		// ---- Runtime -------------------------------------------------
		if (ImGui::BeginTabItem("Runtime")) {

			ImGui::Text("Enter Time   : %.2f", currentEnterTime_);
			ImGui::Text("Queued State : %s", queued_ ? EnumAdapter<PlayerState>::ToString(*queued_) : "None");

			ImGui::SeparatorText("Parry");

			ImGui::Text(std::format("active: {}", parrySession_.active).c_str());
			ImGui::Text(std::format("reserved: {}", parrySession_.reserved).c_str());
			ImGui::Text(std::format("total: {}", parrySession_.total).c_str());

			ImGui::EndTabItem();
		}

		// ---- States --------------------------------------------------
		if (ImGui::BeginTabItem("States")) {
			ImGui::BeginChild("StateList", ImVec2(140, 0), true);
			for (uint32_t i = 0; i < EnumAdapter<PlayerState>::GetEnumCount(); ++i) {

				bool selected = (editingStateIndex_ == static_cast<int>(i));
				if (ImGui::Selectable(EnumAdapter<PlayerState>::GetEnumName(i), selected)) {

					editingStateIndex_ = static_cast<int>(i);
				}
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("StateDetail", ImVec2(0, 0), true);
			if (auto* st = states_[static_cast<PlayerState>(editingStateIndex_)].get()) {
				st->ImGui(owner);
			}
			ImGui::EndChild();

			ImGui::EndTabItem();
		}

		// ---- Conditions ---------------------------------------------
		if (ImGui::BeginTabItem("Conditions")) {
			ImGui::Combo("Edit##cond-state", &comboIndex_,
				EnumAdapter<PlayerState>::GetEnumArray().data(),
				static_cast<int>(EnumAdapter<PlayerState>::GetEnumCount()));

			PlayerState state = static_cast<PlayerState>(comboIndex_);
			PlayerStateCondition& cond = conditions_[state];

			ImGui::Checkbox("isArmor", &cond.isArmor);
			ImGui::Checkbox("enableInARowForceState", &cond.enableInARowForceState);
			ImGui::DragFloat("CoolTime", &cond.coolTime, 0.01f, 0.0f);
			ImGui::DragFloat("InputWindow", &cond.chainInputTime, 0.01f, 0.0f);

			// Allowed / Interruptable をテーブルで
			if (ImGui::BeginTable("CondTable", 3, ImGuiTableFlags_Borders)) {
				ImGui::TableSetupColumn("State");
				ImGui::TableSetupColumn("Allowed");
				ImGui::TableSetupColumn("Interrupt");
				ImGui::TableHeadersRow();

				for (int i = 0; i < EnumAdapter<PlayerState>::GetEnumCount(); ++i) {
					PlayerState s = static_cast<PlayerState>(i);

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::TextUnformatted(EnumAdapter<PlayerState>::GetEnumName(i));

					// ---- Allowed 列 -------------------------------------------------
					ImGui::TableNextColumn();
					{
						bool allowed = std::ranges::find(cond.allowedPreState, s)
							!= cond.allowedPreState.end();
						std::string id = "##allow_" + std::to_string(i);
						if (ImGui::Checkbox(id.c_str(), &allowed)) {
							if (allowed) {
								cond.allowedPreState.push_back(s);
							} else {
								std::erase(cond.allowedPreState, s);
							}
						}
					}

					// ---- Interrupt 列 -----------------------------------------------
					ImGui::TableNextColumn();
					{
						bool intr = std::ranges::find(cond.interruptableBy, s)
							!= cond.interruptableBy.end();
						std::string id = "##intr_" + std::to_string(i);
						if (ImGui::Checkbox(id.c_str(), &intr)) {
							if (intr) {
								cond.interruptableBy.push_back(s);
							} else {
								std::erase(cond.interruptableBy, s);
							}
						}
					}
				}
				ImGui::EndTable();
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void PlayerStateController::ApplyJson() {

	// state
	{
		Json data;
		if (!JsonAdapter::LoadCheck(kStateJsonPath_, data)) {
			return;
		}

		for (auto& [state, ptr] : states_) {

			const auto& key = EnumAdapter<PlayerState>::ToString(state);
			if (!data.contains(key)) {
				continue;
			}
			ptr->ApplyJson(data[key]);
		}

		if (!data.contains("Conditions")) {
			return;
		}
		const Json& condRoot = data["Conditions"];
		for (auto& [state, ptr] : states_) {

			const auto& key = EnumAdapter<PlayerState>::ToString(state);
			if (!condRoot.contains(key)) {
				continue;
			}

			PlayerStateCondition condition{};
			condition.FromJson(condRoot[key]);
			conditions_[state] = std::move(condition);
		}
	}
}

void PlayerStateController::SaveJson() {

	Json data;
	for (auto& [state, ptr] : states_) {
		if (state == PlayerState::None) {
			continue;
		}

		ptr->SaveJson(data[EnumAdapter<PlayerState>::ToString(state)]);
	}

	Json& condRoot = data["Conditions"];
	for (auto& [state, cond] : conditions_) {
		if (state == PlayerState::None) {
			continue;
		}

		cond.ToJson(condRoot[EnumAdapter<PlayerState>::ToString(state)]);
	}

	JsonAdapter::Save(kStateJsonPath_, data);
}