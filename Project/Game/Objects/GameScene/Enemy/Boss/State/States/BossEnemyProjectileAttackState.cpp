#include "BossEnemyProjectileAttackState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>

//============================================================================
//	BossEnemyProjectileAttackState classMethods
//============================================================================

void BossEnemyProjectileAttackState::BulletCollision::Init() {

	// 衝突初期化
	collider = std::make_unique<Collider>();
	// 球で追加
	CollisionBody* body = collider->AddCollider(CollisionShape::Sphere());
	// タイプ設定
	body->SetType(ColliderType::Type_BossWeapon);
	body->SetTargetType(ColliderType::Type_Player);
	// 判定設定
	CollisionShape::Sphere sphere = CollisionShape::Sphere::Default();
	sphere.radius = 4.0f;
	body->SetShape(sphere);

	// その他デフォで初期化
	isActive = false;
	moveTimer.Reset();
	transform.Init();
	// 絶対に当たらない場所で初期化
	transform.translation = collisionSafePos_;
	startPos = collisionSafePos_;
	targetPos = collisionSafePos_;
}

BossEnemyProjectileAttackState::BossEnemyProjectileAttackState(uint32_t phaseCount) {

	// エフェクトの初期化
	// 発生起動エフェクト
	launchEffect_ = std::make_unique<EffectGroup>();
	launchEffect_->Init("launchProjectileEffect", "BossEnemyEffect");
	launchEffect_->LoadJson("GameEffectGroup/BossEnemy/bossEnemylaunchProjectileEffect.json");
	// 弾エフェクト
	for (auto& effect : bulletEffects_) {

		effect = std::make_unique<EffectGroup>();
		effect->Init("bossEnemyProjectileBulletEffect", "BossEnemyEffect");
		effect->LoadJson("GameEffectGroup/BossEnemy/bossEnemyProjectileBulletEffect.json");
	}
	// 弾の衝突判定初期化
	for (auto& bullet : bulletColliders_) {

		bullet.Init();
	}

	for (uint32_t index = 0; index < phaseCount + 1; ++index) {

		// フェーズに応じた弾の数
		// 3から5だけ
		phaseBulletCounts_.emplace_back(std::clamp(kMinBulletCount_ + index * 2,
			kMinBulletCount_, kMaxBulletCount_));
	}
	canExit_ = false;
}

void BossEnemyProjectileAttackState::Enter(BossEnemy& bossEnemy) {

	// アニメーションを再生
	bossEnemy.SetNextAnimation("bossEnemy_projectileAttack", false, nextAnimDuration_);

	// 初期状態を設定
	currentState_ = State::Launch;

	// 現在のフェーズインデックスを取得
	// エディター操作中ならエディターで設定したインデックスを使用する
	currentPhaseIndex_ = isEditMode_ ? editingPhase_ : bossEnemy.GetCurrentPhaseIndex();

	// 発生起動エフェクト前処理
	BeginLaunchPhase(bossEnemy);
}

void BossEnemyProjectileAttackState::Update(BossEnemy& bossEnemy) {

	// 処理中は常にプレイヤーの方を向くようにしておく
	LookTarget(bossEnemy, player_->GetTranslation());

	// 状態に応じて更新
	switch (currentState_) {
	case BossEnemyProjectileAttackState::State::Launch:

		// 発生起動更新
		UpdateLaunch();
		break;
	case BossEnemyProjectileAttackState::State::Attack:

		// 攻撃更新
		UpdateAttack(bossEnemy);
		break;
	}
}

void BossEnemyProjectileAttackState::UpdateLaunch() {

	// 発生時間を更新する
	launchTimer_.Update();

	// 経過進捗で等間隔に発生させる
	uint32_t count = phaseBulletCounts_[currentPhaseIndex_];
	for (uint32_t i = 0; i < count; ++i) {

		// 発生していなければ
		if (!launchEmited_[i]) {

			// i番目の時間を計算
			float t = static_cast<float>(i + 1) / static_cast<float>(count);
			// 発生時間に達していたら発生させる
			if (t <= launchTimer_.t_) {

				// 発生起動エフェクト発生
				launchEffect_->Emit(launchPositions_[launchIndices_[i]]);
				// 発生済み
				launchEmited_[i] = true;
			}
		}
	}

	// 時間経過後弾を発生させる
	if (launchTimer_.IsReached()) {

		// 次の状態へ
		currentState_ = State::Attack;

		// 攻撃エフェクト前処理
		// 発生済みフラグをリセット
		bulletEmited_.assign(phaseBulletCounts_[currentPhaseIndex_], false);
	}
}

void  BossEnemyProjectileAttackState::UpdateAttack(const BossEnemy& bossEnemy) {

	// 弾の数と一発の弾の攻撃時間を目標時間にする
	uint32_t count = phaseBulletCounts_[currentPhaseIndex_];
	attackTimer_.Update(bulletAttackDuration_ * static_cast<float>(count));

	// プレイヤーの座標
	Vector3 playerPos = player_->GetTranslation();
	for (uint32_t i = 0; i < count; ++i) {

		// 発生していなければ
		if (!bulletEmited_[i]) {

			// i番目の時間を計算
			float t = static_cast<float>(i + 1) / static_cast<float>(count);
			if (t <= attackTimer_.t_) {

				// 目標への向き
				Vector3 direction = Vector3(playerPos - bossEnemy.GetTranslation()).Normalize();
				// 目標座標からのオフセットを加える
				Vector3 target = playerPos + direction * targetDistance_;

				// 発生座標
				Vector3 start = launchPositions_[launchIndices_[i]];

				// 弾エフェクト発生
				bulletEffects_[i]->Emit(start);
				// 発生位置、目標座標を設定
				std::vector<Vector3> keys = { start, target };
				bulletEffects_[i]->SetKeyframePath(bulletParticleNodeKey_, keys);
				// 発生済み
				bulletEmited_[i] = true;

				// 衝突判定の設定
				BulletCollision& bulletCollider = bulletColliders_[i];
				// アクティブ状態にして補間を開始させる
				bulletCollider.isActive = true;
				// 補間座標の設定
				bulletCollider.startPos = start;
				bulletCollider.targetPos = target;
			}
		}
	}

	// 時間経過後状態を終了する
	if (attackTimer_.IsReached()) {

		canExit_ = true;
	}
}

void BossEnemyProjectileAttackState::BeginLaunchPhase(BossEnemy& bossEnemy) {

	// 発生位置を設定する
	SetLaunchPositions(bossEnemy, currentPhaseIndex_);
	// 発生順序のインデックスを設定する
	SetLeftToRightIndices(bossEnemy);
	// 発生済みフラグをリセット
	launchEmited_.assign(phaseBulletCounts_[currentPhaseIndex_], false);
}

void BossEnemyProjectileAttackState::SetLeftToRightIndices(const BossEnemy& bossEnemy) {

	launchIndices_.clear();
	for (uint32_t i = 0; i < static_cast<uint32_t>(launchPositions_.size()); ++i) {

		launchIndices_.emplace_back(static_cast<int32_t>(i));
	}

	// 中心
	Vector3 center = bossEnemy.GetTranslation();
	center.y = launchTopPosY_;

	// y軸の向きのベクトルを取得
	Vector3 forward = bossEnemy.GetTransform().GetForward();
	forward.y = 0.0f;
	forward = forward.Normalize();
	Vector3 right = Vector3(forward.z, 0.0f, -forward.x);

	// 座標を方向ベクトルに射影して短い方からソートする
	std::sort(launchIndices_.begin(), launchIndices_.end(),
		[&](uint32_t a, uint32_t b) {

			float projectionA = Vector3::Dot(launchPositions_[a] - center, right);
			float projectionB = Vector3::Dot(launchPositions_[b] - center, right);
			return projectionA < projectionB; });
}

void BossEnemyProjectileAttackState::SetLaunchPositions(const BossEnemy& bossEnemy, int32_t phaseIndex) {

	// 対象フェーズの弾数
	phaseIndex = std::clamp(phaseIndex, 0, static_cast<int32_t>(phaseBulletCounts_.size() - 1));
	launchPositions_.clear();

	// 中心位置、Y座標は固定
	Vector3 center = bossEnemy.GetTranslation();
	center.y = launchTopPosY_;

	// フェーズの左右段数
	uint32_t half = phaseBulletCounts_[phaseIndex] / 2;

	// y軸の向きのベクトルを取得
	Vector3 forward = bossEnemy.GetTransform().GetForward();
	forward.y = 0.0f;
	forward = forward.Normalize();
	Vector3 right = Vector3(forward.z, 0.0f, -forward.x);
	// 絶対値のxオフセット、y,zはそのまま
	Vector3 step = Vector3(std::fabs(launchOffsetPos_.x), launchOffsetPos_.y, launchOffsetPos_.z);

	// 真ん中の発生位置
	launchPositions_.push_back(center);
	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
	for (uint32_t i = 1; i <= half; ++i) {

		// 左右のオフセット距離
		Vector3 offset = step * static_cast<float>(i);
		// 差分
		Vector3 delta = right * offset.x + up * offset.y + forward * offset.z;

		// 左右の発生位置を設定
		launchPositions_.emplace_back(center - right * offset.x + up * offset.y + forward * offset.z);
		launchPositions_.emplace_back(center + right * offset.x + up * offset.y + forward * offset.z);
	}
}

void BossEnemyProjectileAttackState::UpdateAlways([[maybe_unused]] BossEnemy& bossEnemy) {

	// エフェクトの更新
	// 発生起動エフェクト
	launchEffect_->Update();
	// 弾エフェクト
	for (const auto& effect : bulletEffects_) {

		effect->Update();
	}

	// 衝突判定の更新
	// 弾
	for (auto& bullet : bulletColliders_) {

		bullet.Update(bulletLerpDuration_);
	}
}

void BossEnemyProjectileAttackState::BulletCollision::Update(float duration) {

	// アクティブ状態のときに座標を更新する
	if (isActive) {

		LerpTranslation(duration);
	}
	// 衝突判定は常に更新しておく
	collider->UpdateAllBodies(transform);
}

void BossEnemyProjectileAttackState::BulletCollision::LerpTranslation(float duration) {

	// 時間を更新
	moveTimer.Update(duration);

	// 弾の衝突座標を補間する
	transform.translation = Vector3::Lerp(startPos, targetPos, moveTimer.t_);
	transform.UpdateMatrix();

	// 終了次第補間を終了し安全な座標に移す
	if (moveTimer.IsReached()) {

		// リセット
		moveTimer.Reset();
		transform.translation = collisionSafePos_;
		// 非アクティブ状態にする
		isActive = false;
	}
}

void BossEnemyProjectileAttackState::Exit([[maybe_unused]] BossEnemy& bossEnemy) {

	// リセット
	canExit_ = false;
	launchTimer_.Reset();
	attackTimer_.Reset();
}

void BossEnemyProjectileAttackState::ImGui(const BossEnemy& bossEnemy) {

	ImGui::Text("currentState: %s", EnumAdapter<State>::ToString(currentState_));

	ImGui::Checkbox("isEditMode", &isEditMode_);
	ImGui::DragInt("editingPhase", &editingPhase_, 1, 0, static_cast<uint32_t>(phaseBulletCounts_.size() - 1));
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.01f);
	ImGui::DragFloat("nextAnimDuration", &nextAnimDuration_, 0.01f);

	ImGui::SeparatorText("Launch");

	ImGui::DragFloat("launchTopPosY", &launchTopPosY_, 0.01f);
	ImGui::DragFloat3("launchOffsetPos", &launchOffsetPos_.x, 0.01f);

	launchTimer_.ImGui("LaunchTimer");

	ImGui::SeparatorText("Attack");

	ImGui::DragFloat("bulletAttackDuration", &bulletAttackDuration_, 0.01f);
	ImGui::DragFloat("targetDistance", &targetDistance_, 0.01f);
	ImGui::DragFloat("bulletLerpDuration", &bulletLerpDuration_, 0.01f);

	// 座標の更新
	SetLaunchPositions(bossEnemy, isEditMode_ ? editingPhase_ : bossEnemy.GetCurrentPhaseIndex());

	ImGui::SeparatorText("Debug");

	int index = 0;
	for (const auto& emited : bulletEmited_) {

		ImGui::Text("bulletEmited[%d]: %s", index++, emited ? "true" : "false");
	}
	ImGui::Separator();
	for (const auto& bullet : bulletColliders_) {

		ImGui::Text("bulletCollider isActive: %s", bullet.isActive ? "true" : "false");
		ImGui::Text("position: (%.2f, %.2f, %.2f)", bullet.transform.translation.x,
			bullet.transform.translation.y, bullet.transform.translation.z);
	}

	// 現在発生させる位置のデバッグ表示
	for (const auto& pos : launchPositions_) {

		LineRenderer::GetInstance()->DrawSphere(6, 1.0f, pos, Color::Cyan());
	}
}

void BossEnemyProjectileAttackState::ApplyJson(const Json& data) {

	launchTimer_.FromJson(data.value("LaunchTimer", Json()));
	bulletAttackDuration_ = data.value("bulletAttackDuration", 1.0f);
	targetDistance_ = data.value("targetDistance_", 1.0f);
	rotationLerpRate_ = data.value("rotationLerpRate_", 1.0f);
	nextAnimDuration_ = data.value("nextAnimDuration_", 0.16f);
	launchTopPosY_ = data.value("launchTopPosY", 4.0f);
	bulletLerpDuration_ = data.value("bulletLerpDuration_", 0.32f);
	launchOffsetPos_ = Vector3::FromJson(data.value("launchOffsetPos", Json()));
}

void BossEnemyProjectileAttackState::SaveJson(Json& data) {

	launchTimer_.ToJson(data["LaunchTimer"]);
	data["bulletAttackDuration"] = bulletAttackDuration_;
	data["targetDistance_"] = targetDistance_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["nextAnimDuration_"] = nextAnimDuration_;
	data["launchTopPosY"] = launchTopPosY_;
	data["bulletLerpDuration_"] = bulletLerpDuration_;
	data["launchOffsetPos"] = launchOffsetPos_.ToJson();
}