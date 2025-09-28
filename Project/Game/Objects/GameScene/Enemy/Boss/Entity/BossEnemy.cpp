#include "BossEnemy.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Input/Input.h>
#include <Engine/Utility/Random/RandomGenerator.h>

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
	animation_->SetAnimationData("bossEnemy_rushAttack");
	animation_->SetAnimationData("bossEnemy_strongAttack");
	animation_->SetAnimationData("bossEnemy_strongAttackParrySign");
	animation_->SetAnimationData("bossEnemy_stun");
	animation_->SetAnimationData("bossEnemy_stunUpdate");
	animation_->SetAnimationData("bossEnemy_teleport");

	// 右手を親として更新させる
	animation_->SetParentJoint("rightHand");

	// keyEventを設定
	animation_->SetKeyframeEvent("Enemy/Boss/animationEffectKey.json");
	animation_->Update(transform_->matrix.world);

	// アニメーションに合わせて発生させるエフェクト
	animationEffect_ = std::make_unique<BossEnemyAnimationEffect>();
	animationEffect_->Init(*this);
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
	stateController_->Init(*this);
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

	// 状態初期化
	InitState();

	// HUD初期化
	InitHUD();

	// json適応
	ApplyJson();

	// 一度更新しておく
	// HUDの更新
	hudSprites_->SetStatas(stats_);
	hudSprites_->Update(*this);
}

void BossEnemy::SetPlayer(const Player* player) {

	player_ = nullptr;
	player_ = player;

	stateController_->SetPlayer(player);
}

void BossEnemy::SetFollowCamera(const FollowCamera* followCamera) {

	stateController_->SetFollowCamera(followCamera);
	hudSprites_->SetFollowCamera(followCamera);
	animationEffect_->SetFollowCamera(followCamera);
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
	animationEffect_->Update(*this);

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

void BossEnemy::TellParryTiming() {

	// パリィ処理回数を増やす
	++parryTimingTickets_;
}

void BossEnemy::DerivedImGui() {

	// 文字サイズを設定
	ImGui::SetWindowFontScale(0.72f);

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

			animationEffect_->ImGui(*this);
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

	JsonAdapter::Save("Enemy/Boss/initParameter.json", data);
}