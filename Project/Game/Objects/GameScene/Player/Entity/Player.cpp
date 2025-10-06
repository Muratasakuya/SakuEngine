#include "Player.h"

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Engine/Utility/Random/RandomGenerator.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	Player classMethods
//============================================================================

void Player::InitWeapon() {

	// 右手
	rightWeapon_ = std::make_unique<PlayerWeapon>();
	rightWeapon_->Init("playerRightWeapon", "playerRightWeapon", "Player");

	// 左手
	leftWeapon_ = std::make_unique<PlayerWeapon>();
	leftWeapon_->Init("playerLeftWeapon", "playerLeftWeapon", "Player");

	// 武器を手を親として動かす
	if (const auto& hand = GetJointTransform("rightHand")) {

		rightWeapon_->SetParent(*hand);
	}
	if (const auto& hand = GetJointTransform("leftHand")) {

		leftWeapon_->SetParent(*hand);
	}

	// 味方
	ally_ = std::make_unique<GameObject3D>();
	ally_->Init("cube", "playerAlly", "Player");
}

void Player::InitAnimations() {

	// 最初は待機状態で初期化
	animation_->SetPlayAnimation("player_idle", true);

	// animationのデータを設定
	animation_->SetAnimationData("player_walk");
	animation_->SetAnimationData("player_dash");
	animation_->SetAnimationData("player_avoid");
	animation_->SetAnimationData("player_attack_1st");
	animation_->SetAnimationData("player_attack_2nd");
	animation_->SetAnimationData("player_attack_3rd");
	animation_->SetAnimationData("player_attack_4th");
	animation_->SetAnimationData("player_skilAttack_1st");
	animation_->SetAnimationData("player_skilAttack_2nd");
	animation_->SetAnimationData("player_skilAttack_3rd");
	animation_->SetAnimationData("player_stunAttack");
	animation_->SetAnimationData("player_parry");

	// 両手を親として更新させる
	animation_->SetParentJoint("rightHand");
	animation_->SetParentJoint("leftHand");

	// keyEventを設定
	animation_->SetKeyframeEvent("Player/animationEffectKey.json");
	animation_->Update(transform_->matrix.world);

	// アニメーションに合わせて発生させるエフェクト
	animationEffect_ = std::make_unique<PlayerAnimationEffect>();
	animationEffect_->Init(*this);
}

void Player::InitCollision() {

	// OBBで設定
	CollisionBody* body = bodies_.emplace_back(Collider::AddCollider(CollisionShape::OBB().Default()));
	bodyOffsets_.emplace_back(CollisionShape::OBB().Default());

	// タイプ設定
	body->SetType(ColliderType::Type_Player);
	body->SetTargetType(ColliderType::Type_BossEnemy);

	// 衝突を管理するクラスを初期化
	attackCollision_ = std::make_unique<PlayerAttackCollision>();
	attackCollision_->Init();
}

void Player::InitState() {

	// 初期化、ここで初期状態も設定
	stateController_ = std::make_unique<PlayerStateController>();
	stateController_->Init(*this);
}

void Player::InitHUD() {

	// HUDの初期化
	hudSprites_ = std::make_unique<PlayerHUD>();
	hudSprites_->Init();

	stunHudSprites_ = std::make_unique<PlayerStunHUD>();
	stunHudSprites_->Init();
}

void Player::SetInitTransform() {

	transform_->scale = initTransform_.scale;
	transform_->eulerRotate = initTransform_.eulerRotate;
	transform_->rotation = initTransform_.rotation;
	transform_->translation = initTransform_.translation;
}

void Player::DerivedInit() {

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
}

void Player::SetBossEnemy(const BossEnemy* bossEnemy) {

	bossEnemy_ = nullptr;
	bossEnemy_ = bossEnemy;

	stateController_->SetBossEnemy(bossEnemy);
}

void Player::SetFollowCamera(FollowCamera* followCamera) {

	stateController_->SetFollowCamera(followCamera);
	hudSprites_->SetFollowCamera(followCamera);
}

void Player::SetReverseWeapon(bool isReverse, PlayerWeaponType type) {

	// 剣の持ち方設定
	if (isReverse) {
		if (type == PlayerWeaponType::Left) {

			leftWeapon_->SetRotation(Quaternion::MakeAxisAngle(
				Vector3(1.0f, 0.0f, 0.0f), pi));
		} else {

			rightWeapon_->SetRotation(Quaternion::MakeAxisAngle(
				Vector3(1.0f, 0.0f, 0.0f), pi));
		}
	} else {
		if (type == PlayerWeaponType::Left) {

			leftWeapon_->SetRotation(Quaternion::MakeAxisAngle(
				Vector3(1.0f, 0.0f, 0.0f), 0.0f));
		} else {

			rightWeapon_->SetRotation(Quaternion::MakeAxisAngle(
				Vector3(1.0f, 0.0f, 0.0f), 0.0f));
		}
	}
}

void Player::ResetWeaponTransform(PlayerWeaponType type) {

	// 武器の位置を元に戻す
	if (type == PlayerWeaponType::Left) {

		leftWeapon_->ApplyJson(cacheJsonData_["LeftWeapon"]);
		if (const auto& hand = GetJointTransform("leftHand")) {

			leftWeapon_->SetParent(*hand);
		}
	} else if (type == PlayerWeaponType::Right) {

		rightWeapon_->ApplyJson(cacheJsonData_["RightWeapon"]);
		if (const auto& hand = GetJointTransform("rightHand")) {

			rightWeapon_->SetParent(*hand);
		}
	}
}

PlayerWeapon* Player::GetWeapon(PlayerWeaponType type) const {

	switch (type) {
	case PlayerWeaponType::Left:

		return leftWeapon_.get();
	case PlayerWeaponType::Right:

		return rightWeapon_.get();
	default:
		return nullptr;
	}
}

int Player::GetDamage() const {

	// 現在の状態に応じたダメージを取得
	PlayerState currentState = stateController_->GetCurrentState();

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

void Player::Update() {

	// スタン状態のチェック
	CheckBossEnemyStun();

	// 状態の更新
	stateController_->SetStatas(stats_);
	stateController_->Update(*this);

	// 武器の更新
	rightWeapon_->Update();
	leftWeapon_->Update();

	// HUDの更新
	hudSprites_->SetStatas(stats_);
	hudSprites_->Update(*this);
	stunHudSprites_->Update();

	// 衝突情報更新
	Collider::UpdateAllBodies(*transform_);
	attackCollision_->Update(*transform_);

	// エフェクトの更新
	animationEffect_->Update(*this);
}

void Player::CheckBossEnemyStun() {

	// スタン中に敵がスタン状態じゃなくなったら更新終了
	if (isStunUpdate_) {
		if (!bossEnemy_->IsCurrentStunState()) {

			isStunUpdate_ = false;
		}
		return;
	}

	// 敵がスタン状態かどうか
	if (!bossEnemy_->IsCurrentStunState()) {
		return;
	}

	// スタン状態になったら状態を切り替え状態に強制的に遷移させる
	isStunUpdate_ = true;
	stateController_->SetForcedState(*this, PlayerState::SwitchAlly);
}

void Player::OnCollisionEnter(const CollisionBody* collisionBody) {

	// パリィ処理中なら攻撃を受けない
	if (stateController_->IsActiveParry()) {
		return;
	}

	// 敵から攻撃を受けた時のみ
	if ((collisionBody->GetType() & (ColliderType::Type_BossWeapon | ColliderType::Type_BossBlade))
		!= ColliderType::Type_None) {

		// ダメージを受ける
		const int damage = bossEnemy_->GetDamage();
		stats_.currentHP = (std::max)(0, stats_.currentHP - damage);

		// HUDに通知
		hudSprites_->SetDamage(damage);
	}
}

void Player::DerivedImGui() {

	ImGui::PushItemWidth(itemWidth_);
	ImGui::SetWindowFontScale(0.8f);

	if (ImGui::BeginTabBar("PlayerTabs")) {

		// ---- Stats ---------------------------------------------------
		if (ImGui::BeginTabItem("Stats")) {

			ImGui::Text(std::format("isStunUpdate: {}", isStunUpdate_).c_str());

			ImGui::Text("HP : %d / %d", stats_.currentHP, stats_.maxHP);
			ImGui::Text("SP : %d / %d", stats_.currentSkilPoint, stats_.maxSkilPoint);

			ImGui::DragInt("Max HP", &stats_.maxHP, 1, 0);
			ImGui::DragInt("Cur HP", &stats_.currentHP, 1, 0, stats_.maxHP);
			ImGui::DragInt("Max SP", &stats_.maxSkilPoint, 1, 0);
			ImGui::DragInt("Cur SP", &stats_.currentSkilPoint, 1, 0, stats_.maxSkilPoint);

			// 各stateダメージの値を調整
			EnumAdapter<PlayerState>::Combo("EditDamage", &editingState_);

			ImGui::SeparatorText(EnumAdapter<PlayerState>::ToString(editingState_));
			ImGui::DragInt("Damage", &stats_.damages[editingState_], 1, 0);
			ImGui::DragInt("DamageRange", &stats_.damageRandomRange, 1, 0);
			ImGui::DragInt("toughness", &stats_.toughness, 1, 0);

			if (ImGui::Button("Reset HP")) {
				stats_.currentHP = stats_.maxHP;
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset SP")) {
				stats_.currentSkilPoint = 0;
			}
			ImGui::EndTabItem();
		}

		// ---- Init ----------------------------------------------------
		if (ImGui::BeginTabItem("Init")) {

			if (ImGui::Button("Save##InitJson")) {
				SaveJson();
			}

			initTransform_.ImGui(itemWidth_);
			Collider::ImGui(itemWidth_);
			ImGui::EndTabItem();
		}

		// ---- State ---------------------------------------------------
		if (ImGui::BeginTabItem("State")) {
			stateController_->ImGui(*this);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("AttackCollision")) {
			attackCollision_->ImGui();
			ImGui::EndTabItem();
		}

		// ---- HUD -----------------------------------------------------
		if (ImGui::BeginTabItem("HUD")) {
			hudSprites_->ImGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("StunHUD")) {
			stunHudSprites_->ImGui();
			ImGui::EndTabItem();
		}

		// ---- Effect ---------------------------------------------------
		if (ImGui::BeginTabItem("Effect")) {

			animationEffect_->ImGui(*this);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::PopItemWidth();
	ImGui::SetWindowFontScale(1.0f);
}

void Player::ApplyJson() {

	if (!JsonAdapter::LoadCheck("Player/initParameter.json", cacheJsonData_)) {
		return;
	}

	initTransform_.FromJson(cacheJsonData_["Transform"]);
	SetInitTransform();

	GameObject3D::ApplyMaterial(cacheJsonData_);
	Collider::ApplyBodyOffset(cacheJsonData_);

	// 武器
	rightWeapon_->ApplyJson(cacheJsonData_["RightWeapon"]);
	leftWeapon_->ApplyJson(cacheJsonData_["LeftWeapon"]);

	// 衝突
	attackCollision_->ApplyJson(cacheJsonData_["AttackCollision"]);

	stats_.maxHP = JsonAdapter::GetValue<int>(cacheJsonData_, "maxHP");
	stats_.maxSkilPoint = JsonAdapter::GetValue<int>(cacheJsonData_, "maxSkilPoint");
	// 初期化時は最大と同じ値にする
	stats_.currentHP = stats_.maxHP;
	stats_.currentSkilPoint = stats_.maxSkilPoint;

	for (const auto& [key, value] : cacheJsonData_["Damages"].items()) {

		PlayerState state = static_cast<PlayerState>(std::stoi(key));
		stats_.damages[state] = value.get<int>();
	}

	stats_.damageRandomRange = JsonAdapter::GetValue<int>(cacheJsonData_, "DamageRandomRange");
	stats_.toughness = JsonAdapter::GetValue<int>(cacheJsonData_, "toughness");
}

void Player::SaveJson() {

	Json data;

	initTransform_.ToJson(data["Transform"]);
	GameObject3D::SaveMaterial(data);
	Collider::SaveBodyOffset(data);

	// 武器
	rightWeapon_->SaveJson(data["RightWeapon"]);
	leftWeapon_->SaveJson(data["LeftWeapon"]);

	// 衝突
	attackCollision_->SaveJson(data["AttackCollision"]);

	data["maxHP"] = stats_.maxHP;
	data["maxSkilPoint"] = stats_.maxSkilPoint;

	for (const auto& [state, value] : stats_.damages) {

		data["Damages"][std::to_string(static_cast<int>(state))] = value;
	}
	data["DamageRandomRange"] = stats_.damageRandomRange;
	data["toughness"] = stats_.toughness;

	JsonAdapter::Save("Player/initParameter.json", data);
}