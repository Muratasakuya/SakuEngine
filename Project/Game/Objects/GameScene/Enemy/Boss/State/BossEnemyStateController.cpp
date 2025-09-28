#include "BossEnemyStateController.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Engine/Utility/Random/RandomGenerator.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

// state
#include <Game/Objects/GameScene/Enemy/Boss/State/States/BossEnemyIdleState.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/BossEnemyTeleportationState.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/BossEnemyStunState.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/BossEnemyFalterState.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/BossEnemyLightAttackState.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/BossEnemyStrongAttackState.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/BossEnemyChargeAttackState.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/BossEnemyRushAttackState.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/States/BossEnemyContinuousAttackState.h>

//============================================================================
//	BossEnemyStateController classMethods
//============================================================================

void BossEnemyStateController::Init(BossEnemy& owner) {

	// 各状態を初期化
	states_.emplace(BossEnemyState::Idle, std::make_unique<BossEnemyIdleState>());
	states_.emplace(BossEnemyState::Teleport, std::make_unique<BossEnemyTeleportationState>());
	states_.emplace(BossEnemyState::Stun, std::make_unique<BossEnemyStunState>(owner));
	states_.emplace(BossEnemyState::Falter, std::make_unique<BossEnemyFalterState>());
	states_.emplace(BossEnemyState::LightAttack, std::make_unique<BossEnemyLightAttackState>());
	states_.emplace(BossEnemyState::StrongAttack, std::make_unique<BossEnemyStrongAttackState>());
	states_.emplace(BossEnemyState::ChargeAttack, std::make_unique<BossEnemyChargeAttackState>());
	states_.emplace(BossEnemyState::RushAttack, std::make_unique<BossEnemyRushAttackState>());
	states_.emplace(BossEnemyState::ContinuousAttack, std::make_unique<BossEnemyContinuousAttackState>());

	// json適応
	ApplyJson();

	// 初期状態を設定
	requested_ = BossEnemyState::Idle;
	ChangeState(owner);

	disableTransitions_ = false;

	// 攻撃予兆
	attackSign_ = std::make_unique<BossEnemyAttackSign>();
	attackSign_->Init();

	// 各状態に攻撃予兆をセット
	for (const auto& state : std::views::values(states_)) {

		state->SetAttackSign(attackSign_.get());
	}
}

void BossEnemyStateController::SetPlayer(const Player* player) {

	// 各状態にplayerをセット
	for (const auto& state : std::views::values(states_)) {

		state->SetPlayer(player);
	}
}

void BossEnemyStateController::SetFollowCamera(const FollowCamera* followCamera) {

	// 各状態にfollowCameraをセット
	for (const auto& state : std::views::values(states_)) {

		state->SetFollowCamera(followCamera);
	}
}

void BossEnemyStateController::Update(BossEnemy& owner) {

	if (disableTransitions_) {

		// 常に更新する値
		for (const auto& state : std::views::values(states_)) {

			state->UpdateAlways(owner);
		}
		return;
	}

	// 状態切り替えの設定処理
	UpdatePhase();
	CheckStunToughness();
	UpdateStateTimer();

	// 何か設定されて入れば状態遷移させる
	if (requested_.has_value()) {

		ChangeState(owner);
	}

	// 現在の状態を更新
	if (BossEnemyIState* currentState = states_[current_].get()) {

		currentState->Update(owner);
	}

	// 常に更新する値
	for (const auto& state : std::views::values(states_)) {

		state->UpdateAlways(owner);
	}

	// 攻撃予兆の更新処理
	attackSign_->Update();

	if (stateTable_.phases.back().comboIndices.size() == 0) {
		int a = 0;
		++a;
	}
}

void BossEnemyStateController::UpdatePhase() {

	if (stats_.maxHP == 0) {
		return;
	}

	// 現在のHP割合
	uint32_t hpRate = (stats_.currentHP * 100) / stats_.maxHP;

	// HP割合に応じて現在のフェーズを計算して設定
	currentPhase_ = 0;
	for (uint32_t threshold : stats_.hpThresholds) {
		if (hpRate < threshold) {

			// 閾値以下ならフェーズを進める
			++currentPhase_;
		}
	}

	// phaseが切り替わったらリセットする
	if (prevPhase_ != currentPhase_) {

		prevPhase_ = currentPhase_;
		currentComboIndex_ = 0;
		currentSequenceIndex_ = 0;
		stateTimer_ = 0.0f;

		// 最後のphaseの時
		if (currentPhase_ + 1 == stateTable_.phases.size()) {

			// 強制遷移先を設定
			forcedState_ = BossEnemyState::RushAttack;
		} else {

			forcedState_.reset();
		}
	}
}

void BossEnemyStateController::CheckStunToughness() {

	// 靭性値が最大になったらスタン状態にする
	if (stats_.currentDestroyToughness == stats_.maxDestroyToughness) {

		// 強制遷移先を設定
		forcedState_ = BossEnemyState::Stun;
	}
}

void BossEnemyStateController::UpdateStateTimer() {

	// 現在のフェーズの時間を更新
	const auto& phase = stateTable_.phases[currentPhase_];
	BossEnemyIState* state = states_[current_].get();

	// 遷移不可
	if (disableTransitions_) {
		requested_.reset();
		stateTimer_ = 0.0f;
		return;
	}

	if (forcedState_.has_value()) {

		requested_ = *forcedState_;
		forcedState_.reset();
		currentComboSlot_ = 0;
		currentComboIndex_ = 0;
		prevComboIndex_ = 0;
		currentSequenceIndex_ = 0;
		stateTimer_ = 0.0f;
		return;
	}

	// 遷移可能状態になったら時間を進めて遷移させる
	if (state->GetCanExit()) {

		// 強制遷移先が設定されていればその状態に遷移させる
		if (forcedState_.has_value()) {

			requested_ = *forcedState_;
			forcedState_.reset();
			currentComboSlot_ = 0;
			prevComboIndex_ = currentComboIndex_;
			currentSequenceIndex_ = 0;
			stateTimer_ = 0.0f;
			return;
		}

		// 攻撃したかどうか
		const bool isAttack =
			(current_ == BossEnemyState::LightAttack) ||
			(current_ == BossEnemyState::StrongAttack) ||
			(current_ == BossEnemyState::ChargeAttack) ||
			(current_ == BossEnemyState::RushAttack) ||
			(current_ == BossEnemyState::ContinuousAttack);
		// 攻撃状態空の遷移でかつ強制遷移するなら
		if (isAttack && phase.autoIdleAfterAttack) {

			// 強制的に待機状態にする
			requested_ = BossEnemyState::Idle;
			currentComboSlot_ = 0;
			prevComboIndex_ = currentComboIndex_;
			currentSequenceIndex_ = 0;
			stateTimer_ = 0.0f;
			return;
		}

		stateTimer_ += GameTimer::GetDeltaTime();
		if (stateTimer_ >= phase.nextStateDuration) {

			ChooseNextState(phase);
			stateTimer_ = 0.0f;
		}
	}
	// 遷移できない状態
	else {
		stateTimer_ = 0.0f;
	}
}

void BossEnemyStateController::ChangeState(BossEnemy& owner) {

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
	current_ = requested_.value();

	if (current_ == BossEnemyState::Teleport) {

		// テレポートの種類を設定
		auto* teleport = static_cast<BossEnemyTeleportationState*>(states_[BossEnemyState::Teleport].get());
		teleport->SetTeleportType(stateTable_.combos[currentComboIndex_].teleportType);
	}

	// 次の状態を初期化する
	if (auto* currentState = states_[current_].get()) {

		currentState->Enter(owner);
	}
	owner.GetAttackCollision()->SetEnterState(current_);
}

void BossEnemyStateController::ChooseNextState(const BossEnemyPhase& phase) {

	// 新しいコンボを抽選して設定
	const bool startingNewCombo = (currentSequenceIndex_ == 0);
	if (startingNewCombo) {
		if (phase.comboIndices.empty()) return;

		int tryCount = 0;
		// 同じコンボは連続で選択できないようにする
		// tryCountが6を超えたらその状態にする
		do {
			currentComboSlot_ = (phase.comboIndices.size() == 1) ? 0
				: RandomGenerator::Generate(0, int(phase.comboIndices.size() - 1));

			currentComboIndex_ = std::clamp(phase.comboIndices[currentComboSlot_],
				0, int(stateTable_.combos.size() - 1));
			++tryCount;
		} while (currentComboIndex_ == prevComboIndex_ &&
			!stateTable_.combos[currentComboIndex_].allowRepeat && tryCount < 6);

		prevComboIndex_ = currentComboIndex_;
		currentSequenceIndex_ = 0;
	}

	// 次に再生する状態を取得
	const BossEnemyCombo& combo = stateTable_.combos[currentComboIndex_];
	uint32_t sequenceIndex = currentSequenceIndex_;
	BossEnemyState next = combo.sequence[sequenceIndex];

	if (!startingNewCombo && !combo.allowRepeat && next == current_) {

		sequenceIndex = (sequenceIndex + 1) % combo.sequence.size();
		next = combo.sequence[sequenceIndex];
	}

	// indexを次に進める
	currentSequenceIndex_ = sequenceIndex + 1;
	if (combo.sequence.size() <= currentSequenceIndex_) {
		currentSequenceIndex_ = 0;
	}

	requested_ = next;
}

void BossEnemyStateController::SyncPhaseCount() {

	const size_t required = stats_.hpThresholds.size() + 1;
	// 足りなければ追加
	while (stateTable_.phases.size() < required) {

		stateTable_.phases.emplace_back(BossEnemyPhase{});
	}
	// 余っていれば削る
	if (stateTable_.phases.size() > required) {

		stateTable_.phases.resize(required);
	}
}

void BossEnemyStateController::DrawHighlighted(bool highlight, const ImVec4& col, const std::function<void()>& draw) {

	if (highlight) {

		ImGui::PushStyleColor(ImGuiCol_Text, col);
	}

	draw();
	if (highlight) {

		ImGui::PopStyleColor();
	}
}

void BossEnemyStateController::ImGui(const BossEnemy& bossEnemy) {

	if (ImGui::Button("SaveJson...stateParameter.json")) {

		SaveJson();
	}

	// 各stateの値を調整
	EnumAdapter<BossEnemyState>::Combo("EditState", &editingState_);
	ImGui::SeparatorText(EnumAdapter<BossEnemyState>::ToString(editingState_));
	if (const auto& state = states_[editingState_].get()) {

		state->ImGui(bossEnemy);
	}
}

void BossEnemyStateController::EditStateTable() {

	// editorParameters
	const ImVec2 buttonSize = ImVec2(136.0f, 30.0f * 0.72f);

	int currentComboID = -1;
	const auto& curPhaseCombos = stateTable_.phases[currentPhase_].comboIndices;
	if (!curPhaseCombos.empty() && currentComboSlot_ < curPhaseCombos.size()) {
		currentComboID = curPhaseCombos[currentComboSlot_];
	}

	//--------------------------------------------------------------------
	// 概要表示
	//--------------------------------------------------------------------

	ImGui::Checkbox("disableTransitions", &disableTransitions_);
	ImGui::Text("currentState: %s", EnumAdapter<BossEnemyState>::GetEnumName(static_cast<uint32_t>(current_)));

	if (ImGui::Button("SaveJson...stateParameter.json")) {

		SaveJson();
	}
	ImGui::Separator();

	//--------------------------------------------------------------------
	// ComboListテーブル
	//--------------------------------------------------------------------

	ImGui::SeparatorText("Edit Combo");

	// settings
	const ImVec4 headerColor = ImGui::GetStyleColorVec4(ImGuiCol_Header);
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, headerColor);
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, headerColor);

	if (ImGui::Button("CreateCombo")) {
		stateTable_.combos.emplace_back(BossEnemyCombo{});
	}

	if (ImGui::BeginTable("##ComboList", 5, ImGuiTableFlags_BordersInner)) {

		ImGui::TableSetupColumn("Combo");     // 0
		ImGui::TableSetupColumn("Sequence");  // 1
		ImGui::TableSetupColumn("Repeat");    // 2
		ImGui::TableSetupColumn("Teleport");  // 3
		ImGui::TableSetupColumn("AddState");  // 4
		ImGui::TableHeadersRow();

		for (int comboIdx = 0; comboIdx < static_cast<int>(stateTable_.combos.size()); ++comboIdx) {

			auto& combo = stateTable_.combos[comboIdx];
			ImGui::PushID(comboIdx);
			ImGui::TableNextRow();

			//----------------------------------------------------------------
			//  列0: Combo名
			//----------------------------------------------------------------

			ImGui::TableNextColumn();
			const std::string comboLabel = "Combo" + std::to_string(comboIdx);
			bool isCurrentCombo = (comboIdx == currentComboID);

			ImGui::AlignTextToFramePadding();
			DrawHighlighted(isCurrentCombo, kHighlight, [&] {
				ImGui::Selectable(("Combo" + std::to_string(comboIdx)).c_str()); });

			if (ImGui::BeginDragDropSource()) {

				ImGui::SetDragDropPayload("ComboIdx", &comboIdx, sizeof(int));
				ImGui::Text("%s", comboLabel.c_str());
				ImGui::EndDragDropSource();
			}

			//----------------------------------------------------------------
			// 列1: Sequence
			//----------------------------------------------------------------
			ImGui::TableNextColumn();
			for (size_t seqIdx = 0; seqIdx < combo.sequence.size();) {

				const int stateId = static_cast<int>(combo.sequence[seqIdx]);
				bool isCurrentState = isCurrentCombo && (combo.sequence[seqIdx] == current_);

				ImGui::PushID(static_cast<int>(seqIdx));

				bool clicked = false;
				DrawHighlighted(isCurrentState, kHighlight, [&] {
					clicked = ImGui::Button(EnumAdapter<BossEnemyState>::GetEnumName(stateId), buttonSize); });
				if (clicked) {

					combo.sequence.erase(combo.sequence.begin() + seqIdx);
					currentSequenceIndex_ = std::min<uint32_t>(currentSequenceIndex_,
						combo.sequence.empty() ? 0u : uint32_t(combo.sequence.size() - 1));

					ImGui::PopID();
					continue;
				} else {
					if (ImGui::BeginDragDropSource()) {

						const int payload = static_cast<int>(seqIdx);
						ImGui::SetDragDropPayload("SeqReorder", &payload, sizeof(int));
						ImGui::Text("%s", EnumAdapter<BossEnemyState>::GetEnumName(stateId));
						ImGui::EndDragDropSource();
					}
					if (ImGui::BeginDragDropTarget()) {
						if (auto* payload = ImGui::AcceptDragDropPayload("SeqReorder")) {

							const int fromIdx = *static_cast<const int*>(payload->Data);
							std::swap(combo.sequence[fromIdx], combo.sequence[seqIdx]);
						}
						ImGui::EndDragDropTarget();
					}

					// → 区切り矢印
					ImGui::SameLine();
					if (seqIdx < combo.sequence.size() - 1) {

						ImGui::TextUnformatted("-");
						ImGui::SameLine();
					}

					// 削除しなかったときのみ進める
					++seqIdx;
				}

				ImGui::PopID();
			}

			//----------------------------------------------------------------
			// 列2: Repeat
			//----------------------------------------------------------------

			ImGui::TableNextColumn();
			ImGui::Checkbox("##allowRepeat", &combo.allowRepeat);

			//----------------------------------------------------------------
			// 列3: Teleport
			//----------------------------------------------------------------

			ImGui::TableNextColumn();

			ImGui::PushItemWidth(buttonSize.x);
			EnumAdapter<BossEnemyTeleportType>::Combo("##TeleportType", &combo.teleportType);

			ImGui::PopItemWidth();

			//----------------------------------------------------------------
			// 列4: AddStateドロップダウン
			//----------------------------------------------------------------
			ImGui::TableNextColumn();
			{
				ImGui::PushItemWidth(buttonSize.x);

				static BossEnemyState selectedState = BossEnemyState::Idle;
				const std::string addLabel = "##" + std::to_string(comboIdx);
				if (EnumAdapter<BossEnemyState>::Combo(addLabel.c_str(), &selectedState)) {

					const BossEnemyState newState = static_cast<BossEnemyState>(selectedState);
					if (std::ranges::find(combo.sequence, newState) == combo.sequence.end()) {

						combo.sequence.push_back(newState);
					}
				}

				ImGui::PopItemWidth();
			}
			ImGui::PopID();
		}
		ImGui::EndTable();
	}
	ImGui::PopStyleColor(2);

	ImGui::SeparatorText("Edit Phase");

	//--------------------------------------------------------------------
	// Phasesテーブル
	//--------------------------------------------------------------------

	float duration = stateTable_.phases[currentPhase_].nextStateDuration;
	float progress = std::clamp(stateTimer_ / duration, 0.0f, 1.0f);
	ImGui::ProgressBar(progress, ImVec2(200.0f, 0.0f));

	// 数値表示
	ImGui::SameLine();
	ImGui::Text("%.3f / %.3f", stateTimer_, duration);

	// settings
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, headerColor);
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, headerColor);

	// 必要フェーズ数をHP閾値に合わせて調整
	SyncPhaseCount();

	if (ImGui::BeginTable("##Phases", 5, ImGuiTableFlags_BordersInner)) {

		ImGui::TableSetupColumn("Phase");
		ImGui::TableSetupColumn("Duration");
		ImGui::TableSetupColumn("AutoIdle");
		ImGui::TableSetupColumn("Combos");
		ImGui::TableSetupColumn("AddCombo");
		ImGui::TableHeadersRow();

		for (uint32_t phaseIdx = 0; phaseIdx < static_cast<int>(stateTable_.phases.size()); ++phaseIdx) {

			auto& phase = stateTable_.phases[phaseIdx];
			ImGui::PushID(phaseIdx);
			ImGui::TableNextRow();

			//----------------------------------------------------------------
			// 列0: Phase 名
			//----------------------------------------------------------------

			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			DrawHighlighted(phaseIdx == currentPhase_, kHighlight, [&] {
				ImGui::Text("Phase%d", phaseIdx); });

			//----------------------------------------------------------------
			// 列1: Duration
			//----------------------------------------------------------------

			ImGui::TableNextColumn();
			ImGui::DragFloat("##value", &phase.nextStateDuration, 0.01f);

			//----------------------------------------------------------------
			// 列1: AutoIdle
			//----------------------------------------------------------------

			ImGui::TableNextColumn();
			ImGui::Checkbox("##autoIdle", &phase.autoIdleAfterAttack);

			//----------------------------------------------------------------
			// 列3: Phaseが保持するCombo
			//----------------------------------------------------------------

			ImGui::TableNextColumn();
			for (size_t slotIdx = 0; slotIdx < phase.comboIndices.size();) {

				int comboID = phase.comboIndices[slotIdx];
				bool isCurrentCombo = (phaseIdx == currentPhase_) && (comboID == currentComboID);
				const std::string name = "Combo" + std::to_string(comboID);

				ImGui::PushID(static_cast<int>(slotIdx));

				bool clicked = false;
				DrawHighlighted(isCurrentCombo, kHighlight, [&] {
					clicked = ImGui::SmallButton(name.c_str()); });
				if (clicked) {

					phase.comboIndices.erase(phase.comboIndices.begin() + slotIdx);
					if (phaseIdx == currentPhase_) {
						currentComboSlot_ = std::min<uint32_t>(currentComboSlot_,
							phase.comboIndices.empty() ? 0u : uint32_t(phase.comboIndices.size() - 1));
					}
				} else {
					if (ImGui::BeginDragDropSource()) {

						const int payload = static_cast<int>(slotIdx);
						ImGui::SetDragDropPayload("PhaseReorder", &payload, sizeof(int));
						ImGui::Text("%s", name.c_str());
						ImGui::EndDragDropSource();
					}
					if (ImGui::BeginDragDropTarget()) {
						if (auto* payload = ImGui::AcceptDragDropPayload("PhaseReorder")) {

							const int fromIdx = *static_cast<const int*>(payload->Data);
							std::swap(phase.comboIndices[fromIdx],
								phase.comboIndices[slotIdx]);
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();
					++slotIdx;
				}

				ImGui::PopID();
			}

			//----------------------------------------------------------------
			// 列4: AddComboドロップターゲット
			//----------------------------------------------------------------
			ImGui::TableNextColumn();
			ImGui::Dummy(ImVec2(70.0f, 20.0f));

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* pl = ImGui::AcceptDragDropPayload("ComboIdx")) {
					const int comboID = *static_cast<const int*>(pl->Data);
					phase.comboIndices.push_back(comboID);
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::PopID();
		}
		ImGui::EndTable();
	}
	ImGui::PopStyleColor(2);
}

void BossEnemyStateController::ApplyJson() {

	// state
	{
		Json data;
		if (JsonAdapter::LoadCheck(kStateJsonPath_, data)) {
			for (auto& [state, ptr] : states_) {

				const auto& key = EnumAdapter<BossEnemyState>::ToString(state);
				if (!data.contains(key)) {
					continue;
				}
				ptr->ApplyJson(data[key]);
			}
		}
	}

	// tabel
	{
		Json data;
		if (JsonAdapter::LoadCheck(kStateTableJsonPath_, data)) {

			stateTable_.FromJson(data);
		} else {

			stateTable_.combos.clear(); stateTable_.phases.clear();
			// Combo0: Idle
			BossEnemyCombo combo;
			combo.sequence.emplace_back(BossEnemyState::Idle);
			stateTable_.combos.push_back(combo);
			// Phase0がCombo0 を参照
			BossEnemyPhase phase;
			phase.comboIndices.emplace_back(0);
			stateTable_.phases.push_back(phase);
		}
	}
}

void BossEnemyStateController::SaveJson() {

	// state
	{
		Json data;
		for (auto& [state, ptr] : states_) {

			ptr->SaveJson(data[EnumAdapter<BossEnemyState>::ToString(state)]);
		}

		JsonAdapter::Save(kStateJsonPath_, data);
	}

	// table
	{
		Json data;
		stateTable_.ToJson(data);

		JsonAdapter::Save(kStateTableJsonPath_, data);
	}
}
