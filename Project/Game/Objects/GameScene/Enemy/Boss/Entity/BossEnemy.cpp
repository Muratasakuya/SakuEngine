#include "BossEnemy.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Editor/ActionProgress/ActionProgressMonitor.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Input/Input.h>
#include <Engine/Utility/Random/RandomGenerator.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	BossEnemy classMethods
//============================================================================

void BossEnemy::InitWeapon() {

	weapon_ = std::make_unique<BossEnemyWeapon>();
	weapon_->Init("bossEnemyWeapon", "bossEnemyWeapon", "Enemy");

	// 武器を右手を親として動かす
	if (const auto& hand = GetJointTransform("rightHand")) {

		weapon_->SetParent(*hand);
	}
}

void BossEnemy::InitAnimations() {

	// 最初は待機状態で初期化
	animation_->SetPlayAnimation("bossEnemy_idle", true);

	// animationのデータを設定
	animation_->SetAnimationData("bossEnemy_chargeAttack");
	animation_->SetAnimationData("bossEnemy_continuousAttack");
	animation_->SetAnimationData("bossEnemy_falter");
	animation_->SetAnimationData("bossEnemy_lightAttack");
	animation_->SetAnimationData("bossEnemy_lightAttackParrySign");
	animation_->SetAnimationData("bossEnemy_jumpPrepare");
	animation_->SetAnimationData("bossEnemy_jumpAttack");
	animation_->SetAnimationData("bossEnemy_rushAttack");
	animation_->SetAnimationData("bossEnemy_strongAttack");
	animation_->SetAnimationData("bossEnemy_strongAttackParrySign");
	animation_->SetAnimationData("bossEnemy_projectileAttack");
	animation_->SetAnimationData("bossEnemy_stun");
	animation_->SetAnimationData("bossEnemy_stunUpdate");
	animation_->SetAnimationData("bossEnemy_teleport");

	// 右手を親として更新させる
	animation_->SetParentJoint("rightHand");

	// keyEventを設定
	animation_->SetKeyframeEvent("Enemy/Boss/animationEffectKey.json");
	animation_->Update(transform_->matrix.world);

	// 追加
	int id = ActionProgressMonitor::GetInstance()->AddObject("bossEnemy");
	ActionProgressMonitor::GetInstance()->AddOverall(id, "followMove", [this]() -> float {return 0.0f; });
	ActionProgressMonitor::GetInstance()->AddOverall(id, "charge", [this]() -> float {return 0.0f; });
}

void BossEnemy::InitCollision() {

	CollisionBody* body = bodies_.emplace_back(Collider::AddCollider(CollisionShape::OBB().Default()));
	bodyOffsets_.emplace_back(CollisionShape::OBB().Default());

	// タイプ設定
	body->SetType(ColliderType::Type_BossEnemy);
	body->SetTargetType(ColliderType::Type_PlayerWeapon);

	// 衝突を管理するクラスを初期化
	attackCollision_ = std::make_unique<BossEnemyAttackCollision>();
	attackCollision_->Init();
}

void BossEnemy::InitState() {

	// 初期化、ここで初期状態も設定
	stateController_ = std::make_unique<BossEnemyStateController>();
	stateController_->Init(*this, static_cast<uint32_t>(stats_.hpThresholds.size()));
}

void BossEnemy::InitHUD() {

	// HUDの初期化
	hudSprites_ = std::make_unique<BossEnemyHUD>();
	hudSprites_->Init();
	// 最初は表示しない
	hudSprites_->SetDisable();
}

void BossEnemy::SetInitTransform() {

	transform_->scale = initTransform_.scale;
	transform_->eulerRotate = initTransform_.eulerRotate;
	transform_->rotation = initTransform_.rotation;
	transform_->translation = initTransform_.translation;
}

void BossEnemy::CalDistanceToTarget() {

	// 距離レベルを計算
	Vector3 diff = player_->GetTranslation() - transform_->translation;
	// 距離
	const float distance = diff.Length();
	stats_.currentDistanceToTarget = distance;

	// しきい値を距離昇順でソートする
	std::vector<std::pair<float, DistanceLevel>> distancePair;
	distancePair.reserve(stats_.distanceLevels.size());
	for (const auto& [level, radius] : stats_.distanceLevels) {

		distancePair.emplace_back(radius, level);
	}
	// 昇順ソート
	std::sort(distancePair.begin(), distancePair.end(),
		[](const auto& a, const auto& b) { return a.first < b.first; });
	auto it = std::lower_bound(distancePair.begin(), distancePair.end(), distance,
		[](const auto& e, float val) { return e.first < val; });

	// 範囲内ならその距離レベルを設定
	if (it != distancePair.end()) {

		stats_.currentDistanceLevel = it->second;
	}
	// Farよりも遠ければFarにする
	else {

		stats_.currentDistanceLevel = DistanceLevel::Far;
	}
}

void BossEnemy::DebugCommand() {

#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	// キルコマンド
	if (Input::GetInstance()->PushKey(DIK_0)) {
		if (Input::GetInstance()->TriggerKey(DIK_1)) {

			stats_.currentHP = 0;
		}
	}
#endif
}

void BossEnemy::DerivedInit() {

	// 使用する武器を初期化
	InitWeapon();

	// animation初期化、設定
	InitAnimations();

	// collision初期化、設定
	InitCollision();

	// 距離レベル仮初期化
	stats_.distanceLevels.emplace(DistanceLevel::Near, 2.0f);
	stats_.distanceLevels.emplace(DistanceLevel::Middle, 4.0f);
	stats_.distanceLevels.emplace(DistanceLevel::Far, 6.0f);

	// json適応
	ApplyJson();

	// 状態初期化
	InitState();

	// HUD初期化
	InitHUD();

	// 一度更新しておく
	// HUDの更新
	hudSprites_->SetStatas(stats_);
	hudSprites_->Update(*this);
}

void BossEnemy::SetPlayer(Player* player) {

	player_ = nullptr;
	player_ = player;

	stateController_->SetPlayer(player);
}

void BossEnemy::SetFollowCamera(FollowCamera* followCamera) {

	stateController_->SetFollowCamera(followCamera, *this);
	hudSprites_->SetFollowCamera(followCamera);
}

void BossEnemy::SetAlpha(float alpha) {

	// 武器も一緒に設定する
	GameObject3D::SetAlpha(alpha);
	weapon_->SetAlpha(alpha);
}

void BossEnemy::SetCastShadow(bool cast) {

	// 武器も一緒に設定する
	GameObject3D::SetCastShadow(cast);
	weapon_->SetCastShadow(cast);
}

void BossEnemy::SetDecreaseToughnessProgress(float progress) {

	// progressに応じて靭性値を下げる
	stats_.currentDestroyToughness = std::clamp(static_cast<int>(std::lerp(stats_.maxDestroyToughness,
		0, progress)), 0, stats_.maxDestroyToughness);
}

Vector3 BossEnemy::GetWeaponTranslation() const {

	return weapon_->GetTransform().GetWorldPos();
}

bool BossEnemy::IsCurrentStunState() const {

	return stateController_->GetCurrentState() == BossEnemyState::Stun;
}

int  BossEnemy::GetDamage() const {

	BossEnemyState currentState = stateController_->GetCurrentState();

	// ダメージを与えられる状態か確認してから設定
	if (Algorithm::Find(stats_.damages, currentState)) {

		int damage = stats_.damages.at(currentState);
		// ランダムでダメージを設定
		damage = RandomGenerator::Generate(damage - stats_.damageRandomRange,
			damage + stats_.damageRandomRange);
		return damage;
	}
	return 0;
}

bool BossEnemy::IsDead() const {

	return stats_.currentHP == 0;
}

uint32_t BossEnemy::GetCurrentPhaseIndex() const {

	// 現在のHP割合
	uint32_t hpRate = (stats_.currentHP * 100) / stats_.maxHP;

	uint32_t phaseIndex = 0;
	for (uint32_t threshold : stats_.hpThresholds) {
		if (hpRate < threshold) {

			// 閾値以下ならフェーズを進める
			++phaseIndex;
		}
	}
	return phaseIndex;
}

void BossEnemy::Update(GameSceneState sceneState) {

	// シーンの状態に応じた更新処理
	switch (sceneState) {
	case GameSceneState::Start:
		break;
	case GameSceneState::BeginGame:

		// ゲーム開始時の登場演出
		UpdateBeginGame();
		break;
	case GameSceneState::PlayGame:

		// ゲームプレイ中
		UpdatePlayGame();
		break;
	case GameSceneState::EndGame:

		// ゲーム終了
		UpdateEndGame();
		break;
	}
	// シーン状態のチェック
	CheckSceneState(sceneState);
}

void BossEnemy::UpdateBeginGame() {


}

void BossEnemy::UpdatePlayGame() {

	// 閾値のリストの条件に誤りがないかチェック
	// indexNはindexN+1の値より必ず大きい(N=80、N+1=85にはならない)
	if (!stats_.hpThresholds.empty()) {

		std::sort(stats_.hpThresholds.begin(), stats_.hpThresholds.end(), std::greater<int>());
	}

	// 状態処理開始前に距離レベルを決定する
	CalDistanceToTarget();

	// 状態の更新
	stateController_->SetStatas(stats_);
	stateController_->Update(*this);

	// 武器の更新
	weapon_->Update();

	// HUDの更新
	hudSprites_->SetStatas(stats_);
	hudSprites_->Update(*this);

	// 衝突情報更新
	Collider::UpdateAllBodies(*transform_);
	attackCollision_->Update(*transform_);

	// エフェクトの更新
	// エフェクト、エンジン機能変更中...
	//animationEffect_->Update(*this);

	// デバッグ用コマンド
	DebugCommand();
}

void BossEnemy::UpdateEndGame() {
}

void BossEnemy::CheckSceneState(GameSceneState sceneState) {

	// シーンが切り替わったとき
	if (preSceneState_ != sceneState) {
		switch (preSceneState_) {
		case GameSceneState::Start:

			// HUDの表示を行う
			hudSprites_->SetValid();
			break;
		case GameSceneState::BeginGame:
			break;
		case GameSceneState::PlayGame:
			break;
		case GameSceneState::EndGame:
			break;
		}
	}
	preSceneState_ = sceneState;
}

void BossEnemy::OnCollisionEnter(const CollisionBody* collisionBody) {

	// playerからの攻撃を受けた時
	if (collisionBody->GetType() == ColliderType::Type_PlayerWeapon) {

		// ダメージを受ける
		const int damage = player_->GetDamage();
		stats_.currentHP = (std::max)(0, stats_.currentHP - damage);

		// スタン状態じゃないときのみ
		if (stateController_->GetCurrentState() != BossEnemyState::Stun) {

			// 靭性値を増やす
			stats_.currentDestroyToughness = (std::min)(stats_.currentDestroyToughness + player_->GetToughness(),
				stats_.maxDestroyToughness);

			// 靭性値が最大にまで行ったらHUDの表示を消す
			if (stats_.currentDestroyToughness == stats_.maxDestroyToughness) {

				hudSprites_->SetDisable();
			}
		}

		// 怯ませるように状態管理クラスに通知
		stateController_->OnDamaged();
		// HUDに通知
		hudSprites_->SetDamage(damage);
	}
}

bool BossEnemy::ConsumeParryTiming() {

	// 処理回数が0ならfalse
	if (parryTimingTickets_ == 0) {
		return false;
	}
	--parryTimingTickets_;
	return true;
}

void BossEnemy::UpdateFalterCooldown() {

	// 時間を進める
	stats_.reFalterTimer.Update();
	if (stats_.reFalterTimer.IsReached()) {

		// また怯めるようにする
		stats_.currentFalterCount = 0;
		stats_.reFalterTimer.Reset();
	}
}

void BossEnemy::TellParryTiming() {

	// パリィ処理回数を増やす
	++parryTimingTickets_;
}

void BossEnemy::DerivedImGui() {

	// 文字サイズを設定
	ImGui::SetWindowFontScale(0.58f);

	ImGui::SeparatorText("HP");

	ImGui::Text("currentHP: %d / %d", stats_.currentHP, stats_.maxHP);
	ImGui::DragInt("maxHP", &stats_.maxHP, 1, 0);
	ImGui::DragInt("currentHP", &stats_.currentHP, 1, 0, stats_.maxHP);
	if (ImGui::Button("ResetHP")) {

		// HPをリセットする
		stats_.currentHP = stats_.maxHP;
	}

	ImGui::SeparatorText("DestroyToughness");

	ImGui::Text("currentDestroyToughness: %d / %d", stats_.currentDestroyToughness, stats_.maxDestroyToughness);
	ImGui::DragInt("maxDestroyToughness", &stats_.maxDestroyToughness, 1, 0);
	ImGui::DragInt("currentDestroyToughness", &stats_.currentDestroyToughness, 1, 0, stats_.maxDestroyToughness);
	if (ImGui::Button("ResetDestroyToughness")) {

		// 靭性値をリセットする
		stats_.currentDestroyToughness = 0;
	}

	ImGui::SeparatorText("Damage");

	EnumAdapter<BossEnemyState>::Combo("EditDamage", &editingState_);
	ImGui::SeparatorText(EnumAdapter<BossEnemyState>::ToString(editingState_));
	ImGui::DragInt("Damage", &stats_.damages[editingState_], 1, 0);
	ImGui::DragInt("DamageRange", &stats_.damageRandomRange, 1, 0);

	ImGui::SeparatorText("ReloadData");

	if (ImGui::Button("Reload keyEvent")) {

		// keyEventを設定
		animation_->SetKeyframeEvent("Enemy/Boss/animationEffectKey.json");
	}

	ImGui::SeparatorText("Parry");

	ImGui::Text("parryTimingTickets: %d", parryTimingTickets_);
	ImGui::Text(std::format("ConsumeParryTiming: {}", ConsumeParryTiming()).c_str());

	ImGui::SeparatorText("DistanceLevel");

	ImGui::Checkbox("isDrawDistanceLevel", &isDrawDistanceLevel_);

	for (auto& [level, distance] : stats_.distanceLevels) {

		const std::string label = EnumAdapter<DistanceLevel>::ToString(level);
		ImGui::DragFloat(label.c_str(), &distance, 0.1f, 0.0f, 1000.0f);
	}
	// 現在の距離
	ImGui::Text("currentDistanceToTarget: %.2f", stats_.currentDistanceToTarget);
	ImGui::Text("currentDistanceLevel: %s", EnumAdapter<DistanceLevel>::ToString(stats_.currentDistanceLevel));

	// 距離レベルの描画
	if (isDrawDistanceLevel_) {

		// 向き
		Vector3 playerPos = player_->GetTranslation();
		Vector3 enemyPos = transform_->translation;
		// y座標を固定
		playerPos.y = enemyPos.y = 4.0f;
		Vector3 direction = Vector3(playerPos - enemyPos).Normalize();

		// 距離レベルの描画
		// Near
		LineRenderer::GetInstance()->DrawLine3D(enemyPos,
			enemyPos + direction * stats_.distanceLevels[DistanceLevel::Near], Color::Red());
		// Middle
		LineRenderer::GetInstance()->DrawLine3D(enemyPos,
			enemyPos + direction * stats_.distanceLevels[DistanceLevel::Middle], Color::Green());
		// Far
		LineRenderer::GetInstance()->DrawLine3D(enemyPos,
			enemyPos + direction * stats_.distanceLevels[DistanceLevel::Far], Color::Cyan());

		// 今
		LineRenderer::GetInstance()->DrawLine3D(enemyPos,
			enemyPos + direction * stats_.currentDistanceToTarget, Color::Yellow());
	}

	ImGui::Separator();

	if (ImGui::BeginTabBar("BossEnemyTab")) {
		if (ImGui::BeginTabItem("Init")) {

			if (ImGui::Button("SaveJson...initParameter.json")) {

				SaveJson();
			}

			// 閾値の追加、設定処理
			if (ImGui::Button("AddHPThreshold")) {

				stats_.hpThresholds.emplace_back(0);
			}
			if (!stats_.hpThresholds.empty()) {

				std::vector<std::string> phaseLabels;
				std::vector<const char*> labelPtrs;

				phaseLabels.reserve(stats_.hpThresholds.size());
				labelPtrs.reserve(stats_.hpThresholds.size());
				for (size_t i = 0; i < stats_.hpThresholds.size(); ++i) {

					phaseLabels.emplace_back("Phase" + std::to_string(i));
					labelPtrs.push_back(phaseLabels.back().c_str());
				}
				ImGui::Combo("Edit Phase", &selectedPhaseIndex_, labelPtrs.data(), static_cast<int>(labelPtrs.size()));
				ImGui::DragInt("Threshold(%)", &stats_.hpThresholds[selectedPhaseIndex_], 1, 0, 100);
			}

			ImGui::Separator();

			if (ImGui::CollapsingHeader("Transform")) {

				initTransform_.ImGui(itemWidth_);
				SetInitTransform();
			}

			if (ImGui::CollapsingHeader("Collision")) {

				Collider::ImGui(itemWidth_);
			}

			if (ImGui::CollapsingHeader("Falter")) {

				ImGui::Text("currentFalterCount: %d", stats_.currentFalterCount);
				ImGui::DragInt("maxFalterCount", &stats_.maxFalterCount, 1, 0, 256);
				stats_.reFalterTimer.ImGui("ReFalterTimer");

				ImGui::SeparatorText("BlockFalterStates");

				// 追加候補の選択
				static BossEnemyState candidate = BossEnemyState::Idle;
				EnumAdapter<BossEnemyState>::Combo("Add State", &candidate);

				ImGui::SameLine();
				if (ImGui::Button("Add")) {

					// 重複追加防止
					auto it = std::find(stats_.blockFalterStates.begin(),
						stats_.blockFalterStates.end(), candidate);
					if (it == stats_.blockFalterStates.end()) {
						stats_.blockFalterStates.push_back(candidate);
					}
				}

				ImGui::SameLine();
				if (ImGui::Button("ClearAll")) {
					stats_.blockFalterStates.clear();
				}
				if (ImGui::BeginTable("##BlockFalterStatesTable", 2, ImGuiTableFlags_BordersInner)) {

					ImGui::TableSetupColumn("State");
					ImGui::TableSetupColumn("Remove");
					ImGui::TableHeadersRow();
					for (size_t i = 0; i < stats_.blockFalterStates.size();) {

						ImGui::TableNextRow();

						// 列0: 名前表示
						ImGui::TableNextColumn();
						const auto name = EnumAdapter<BossEnemyState>::ToString(stats_.blockFalterStates[i]);
						ImGui::TextUnformatted(name);

						// 列1: 削除ボタン
						ImGui::TableNextColumn();
						ImGui::PushID(static_cast<int>(i));
						const bool removed = ImGui::SmallButton("X");
						ImGui::PopID();

						if (removed) {
							stats_.blockFalterStates.erase(stats_.blockFalterStates.begin() + static_cast<long>(i));
							continue;
						} else {
							++i;
						}
					}
					ImGui::EndTable();
				}
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("AttackCollision")) {

			attackCollision_->ImGui();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("StateParam")) {

			stateController_->ImGui(*this);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("StateTable")) {

			stateController_->EditStateTable();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("HUD")) {

			hudSprites_->ImGui();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Effect")) {

			// エフェクト、エンジン機能変更中...
			//animationEffect_->ImGui(*this);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	// 文字サイズを元に戻す
	ImGui::SetWindowFontScale(1.0f);
}

void BossEnemy::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Enemy/Boss/initParameter.json", data)) {
		return;
	}

	initTransform_.FromJson(data["Transform"]);
	SetInitTransform();

	GameObject3D::ApplyMaterial(data);
	Collider::ApplyBodyOffset(data);

	// 衝突
	attackCollision_->ApplyJson(data["AttackCollision"]);

	stats_.maxHP = JsonAdapter::GetValue<int>(data, "maxHP");
	stats_.maxDestroyToughness = JsonAdapter::GetValue<int>(data, "maxDestroyToughness");
	// 初期化時は最大と同じ値にする
	stats_.currentHP = stats_.maxHP;
	stats_.hpThresholds = JsonAdapter::ToVector<int>(data["hpThresholds"]);

	for (const auto& [key, value] : data["Damages"].items()) {

		BossEnemyState state = static_cast<BossEnemyState>(std::stoi(key));
		stats_.damages[state] = value.get<int>();
	}
	stats_.damageRandomRange = JsonAdapter::GetValue<int>(data, "DamageRandomRange");

	stats_.maxFalterCount = data.value("maxFalterCount", 6);
	stats_.reFalterTimer.FromJson(data.value("ReFalterTimer", Json()));

	if (data.contains("BlockFalterState")) {
		for (const auto& stateData : data["BlockFalterState"]) {

			auto state = EnumAdapter<BossEnemyState>::FromString(stateData);
			stats_.blockFalterStates.push_back(state.value());
		}
	}

	if (data.contains("DistanceLevels")) {
		for (const auto& [key, value] : data["DistanceLevels"].items()) {

			DistanceLevel level = EnumAdapter<DistanceLevel>::FromString(key).value();
			stats_.distanceLevels[level] = value.get<float>();
		}
	}
}

void BossEnemy::SaveJson() {

	Json data;

	initTransform_.ToJson(data["Transform"]);
	GameObject3D::SaveMaterial(data);
	Collider::SaveBodyOffset(data);

	// 衝突
	attackCollision_->SaveJson(data["AttackCollision"]);

	data["maxHP"] = stats_.maxHP;
	data["maxDestroyToughness"] = stats_.maxDestroyToughness;

	data["hpThresholds"] = JsonAdapter::FromVector<int>(stats_.hpThresholds);

	for (const auto& [state, value] : stats_.damages) {

		data["Damages"][std::to_string(static_cast<int>(state))] = value;
	}
	data["DamageRandomRange"] = stats_.damageRandomRange;

	data["maxFalterCount"] = stats_.maxFalterCount;
	stats_.reFalterTimer.ToJson(data["ReFalterTimer"]);
	for (const auto& state : stats_.blockFalterStates) {

		data["BlockFalterState"].push_back(EnumAdapter<BossEnemyState>::ToString(state));
	}

	for (const auto& [level, value] : stats_.distanceLevels) {

		data["DistanceLevels"][EnumAdapter<DistanceLevel>::ToString(level)] = value;
	}

	JsonAdapter::Save("Enemy/Boss/initParameter.json", data);
}