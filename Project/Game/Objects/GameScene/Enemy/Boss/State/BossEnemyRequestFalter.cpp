#include "BossEnemyRequestFalter.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Enemy/Boss/State/BossEnemyStateController.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	BossEnemyRequestFalter classMethods
//============================================================================

void BossEnemyRequestFalter::Init(const BossEnemy* bossEnemy, const Player* player) {

	bossEnemy_ = nullptr;
	bossEnemy_ = bossEnemy;
	player_ = nullptr;
	player_ = player;

	// マップ追加
	for (const auto& state : EnumAdapter<BossEnemyState>::GetEnumArray()) {

		allowFalterBossInfos_.emplace(EnumAdapter<BossEnemyState>::FromString(state).value(), BossEnemyStateInfo{ false });
	}
	for (const auto& state : EnumAdapter<PlayerState>::GetEnumArray()) {

		falterPlayerInfos_.emplace(EnumAdapter<PlayerState>::FromString(state).value(), PlayerStateInfo{ false });
	}

	// カウント初期化
	currentFalterCount_ = 0;
	currentRecoverFalterCount_ = 0;
	disableTransitionActive_ = false;
	disablePlayerState_ = PlayerState::None;

	// json適用
	ApplyJson();
}

void BossEnemyRequestFalter::Update(BossEnemyStateController& stateController) {

	// 状態遷移可なら処理しない
	if (!disableTransitionActive_) {
		return;
	}

	// 現在のプレイヤー状態を取得
	PlayerState current = player_->GetCurrentState();

	// ロック元の状態から変わったら解除する
	if (current != disablePlayerState_) {

		// 遷移を許可する
		stateController.SetDisableTransitions(false);

		// リセット
		disableTransitionActive_ = false;
		disablePlayerState_ = PlayerState::None;
	}
}

bool BossEnemyRequestFalter::Check(BossEnemyStateController& stateController) {

	// 敵とプレイヤーの状態を取得
	BossEnemyState bossState = bossEnemy_->GetCurrentState();
	PlayerState playerState = player_->GetCurrentState();

	auto& bossInfo = allowFalterBossInfos_.at(bossState);
	auto& playerInfo = falterPlayerInfos_.at(playerState);

	if (playerInfo.isDisableState && !disableTransitionActive_) {

		// 状態遷移を不可にする
		stateController.SetDisableTransitions(true);
		disableTransitionActive_ = true;
		disablePlayerState_ = playerState;
	}

	// プレイヤー攻撃が強制的に怯ませるならtrueを返す
	if (bossInfo.isAllow && playerInfo.isForce) {

		// 攻撃カウントをリセットする
		currentRecoverFalterCount_ = 0;
		++currentFalterCount_;
		return true;
	}

	// 怯めないときはfalseを返す
	if (maxFalterCount_ <= currentFalterCount_) {
		return false;
	}

	// ボスの状態が怯み許可ならtrueを返す
	if (bossInfo.isAllow) {
		++currentFalterCount_;
		return true;
	}
	return false;
}

void BossEnemyRequestFalter::IncrementRecoverCount() {

	// 怯み不可になっているときのみ加算する
	// 最大数以下の時は早期リターン
	if (currentFalterCount_ < maxFalterCount_) {
		return;
	}

	// 怯み回数カウントを増やす
	++currentRecoverFalterCount_;

	// 最大数を超えたらまた怯めるようにする
	if (recoverFalterCount_ <= currentRecoverFalterCount_) {

		currentFalterCount_ = 0;
		currentRecoverFalterCount_ = 0;
	}
}

void BossEnemyRequestFalter::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	//---------------------------
	// カウンタ系パラメータ
	//---------------------------
	ImGui::TextUnformatted("Falter Count Settings");

	int maxFalter = static_cast<int>(maxFalterCount_);
	if (ImGui::DragInt("Max Falter Count", &maxFalter, 1.0f, 0, 9999)) {

		if (maxFalter < 0) { maxFalter = 0; }
		maxFalterCount_ = static_cast<uint32_t>(maxFalter);
	}

	int recoverFalter = static_cast<int>(recoverFalterCount_);
	if (ImGui::DragInt("Recover Falter Count", &recoverFalter, 1.0f, 0, 9999)) {

		if (recoverFalter < 0) { recoverFalter = 0; }
		recoverFalterCount_ = static_cast<uint32_t>(recoverFalter);
	}

	ImGui::Separator();

	// 現在値のデバッグ表示
	ImGui::Text("Current Falter Count: %u", currentFalterCount_);
	ImGui::Text("Current Recover Count: %u", currentRecoverFalterCount_);
	if (ImGui::SmallButton("Reset Current Counts")) {
		currentFalterCount_ = 0;
		currentRecoverFalterCount_ = 0;
	}

	ImGui::Separator();

	//---------------------------
	// ボス側の怯み許可状態
	//---------------------------
	if (ImGui::TreeNode("Boss States (Allow Falter)")) {

		for (auto& [state, info] : allowFalterBossInfos_) {

			const char* name = EnumAdapter<BossEnemyState>::ToString(state);
			bool allow = info.isAllow;
			if (ImGui::Checkbox(name, &allow)) {
				info.isAllow = allow;
			}
		}
		ImGui::TreePop();
	}

	ImGui::Separator();

	//---------------------------
	// プレイヤー側の強制怯み状態
	//---------------------------
	if (ImGui::TreeNode("Player States (Force Falter)")) {

		for (auto& [state, info] : falterPlayerInfos_) {

			const char* name = EnumAdapter<PlayerState>::ToString(state);
			std::string label = std::string(name) + " :isForce";
			ImGui::Checkbox(label.c_str(), &info.isForce);
			label = std::string(name) + " :isDisableState";
			ImGui::Checkbox(label.c_str(), &info.isDisableState);
		}
		ImGui::TreePop();
	}
}

void BossEnemyRequestFalter::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Enemy/Boss/requestFalter.json", data)) {
		return;
	}

	// カウンタ系
	if (data.contains("maxFalterCount")) {

		maxFalterCount_ = data["maxFalterCount"].get<uint32_t>();
	}
	if (data.contains("recoverFalterCount")) {

		recoverFalterCount_ = data["recoverFalterCount"].get<uint32_t>();
	}

	// ボスの状態ごとの怯み許可
	if (data.contains("allowFalterBossStates")) {

		const auto& bossStates = data["allowFalterBossStates"];
		for (auto it = bossStates.begin(); it != bossStates.end(); ++it) {

			const std::string& name = it.key();
			auto enumOpt = EnumAdapter<BossEnemyState>::FromString(name);
			if (!enumOpt) {
				continue;
			}
			auto mapIt = allowFalterBossInfos_.find(*enumOpt);
			if (mapIt == allowFalterBossInfos_.end()) {
				continue;
			}

			const auto& node = it.value();

			mapIt->second.isAllow = node.value("isAllow", false);
		}
	}

	// プレイヤーの状態ごとの強制怯み
	if (data.contains("forceFalterPlayerStates")) {

		const auto& playerStates = data["forceFalterPlayerStates"];
		for (auto it = playerStates.begin(); it != playerStates.end(); ++it) {

			const std::string& name = it.key();
			auto enumOpt = EnumAdapter<PlayerState>::FromString(name);
			if (!enumOpt) {
				continue;
			}
			auto mapIt = falterPlayerInfos_.find(*enumOpt);
			if (mapIt == falterPlayerInfos_.end()) {
				continue;
			}

			const auto& node = it.value();

			mapIt->second.isForce = node.value("isForce", false);
			mapIt->second.isDisableState = node.value("isDisableState", false);
		}
	}
}

void BossEnemyRequestFalter::SaveJson() {

	Json data;

	// カウンタ系
	data["maxFalterCount"] = maxFalterCount_;
	data["recoverFalterCount"] = recoverFalterCount_;

	// ボスの状態ごとの怯み許可
	auto& bossStates = data["allowFalterBossStates"];
	for (const auto& [state, info] : allowFalterBossInfos_) {

		const char* name = EnumAdapter<BossEnemyState>::ToString(state);
		bossStates[name]["isAllow"] = info.isAllow;
	}

	// プレイヤーの状態ごとの強制怯み
	auto& playerStates = data["forceFalterPlayerStates"];
	for (const auto& [state, info] : falterPlayerInfos_) {

		const char* name = EnumAdapter<PlayerState>::ToString(state);
		playerStates[name]["isForce"] = info.isForce;
		playerStates[name]["isDisableState"] = info.isDisableState;
	}

	JsonAdapter::Save("Enemy/Boss/requestFalter.json", data);
}