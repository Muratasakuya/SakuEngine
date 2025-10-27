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

BossEnemyProjectileAttackState::BossEnemyProjectileAttackState(uint32_t phaseCount) {

	// エフェクトの初期化
	// 予備動作エフェクト
	preEffect_ = std::make_unique<EffectGroup>();
	preEffect_->Init("preProjectileEffect", "BossEnemyEffect");
	preEffect_->LoadJson("GameEffectGroup/BossEnemy/bossEnemyPreProjectileEffect.json");
	// 発生起動エフェクト
	launchEffect_ = std::make_unique<EffectGroup>();
	launchEffect_->Init("launchProjectileEffect", "BossEnemyEffect");
	launchEffect_->LoadJson("GameEffectGroup/BossEnemy/bossEnemylaunchProjectileEffect.json");
	// 弾エフェクト
	bulletEffect_ = std::make_unique<EffectGroup>();
	bulletEffect_->Init("bossEnemyProjectileBulletEffect", "BossEnemyEffect");
	bulletEffect_->LoadJson("GameEffectGroup/BossEnemy/bossEnemyProjectileBulletEffect.json");

	for (uint32_t index = 0; index < phaseCount + 1; ++index) {

		// 3..5..7..
		uint32_t count = 3 + index * 2;

		// フェーズに応じた弾の数
		phaseBulletCounts_.emplace_back(count);
	}
	canExit_ = false;
}

void BossEnemyProjectileAttackState::Enter(BossEnemy& bossEnemy) {

	// 初期状態を設定
	currentState_ = State::Pre;

	// 予備動作エフェクト開始
	preEffect_->Emit(bossEnemy.GetTranslation());
}

void BossEnemyProjectileAttackState::Update(BossEnemy& bossEnemy) {

	// 処理中は常にプレイヤーの方を向くようにしておく
	LookTarget(bossEnemy, player_->GetTranslation());

	// 状態に応じて更新
	switch (currentState_) {
	case BossEnemyProjectileAttackState::State::Pre:

		// 予備動作更新
		UpdatePre(bossEnemy);
		break;
	case BossEnemyProjectileAttackState::State::Launch:

		// 発生起動更新
		UpdateLaunch(bossEnemy);
		break;
	case BossEnemyProjectileAttackState::State::Attack:

		// 攻撃更新
		UpdateAttack(bossEnemy);
		break;
	}
}

void  BossEnemyProjectileAttackState::UpdatePre(BossEnemy& bossEnemy) {

	// エフェクトの処理が終了したら次の状態に進ませる
	if (preEffect_->IsFinishedAllNode()) {

		// 次の状態へ
		currentState_ = State::Launch;

		// 発生起動エフェクト前処理
		BeginLaunchPhase(bossEnemy);
	}
}

void  BossEnemyProjectileAttackState::UpdateLaunch(BossEnemy& bossEnemy) {


	// 発生時間を更新する
	launchTimer_.Update();

	// 経過進捗で等間隔に発生させる
	uint32_t count = phaseBulletCounts_[editingPhase_];
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
		BeginAttackPhase(bossEnemy);
	}
}

void  BossEnemyProjectileAttackState::UpdateAttack(BossEnemy& bossEnemy) {

	// 弾の数と一発の弾の攻撃時間を目標時間にする
	uint32_t count = phaseBulletCounts_[editingPhase_];
	attackTimer_.Update(bulletAttackDuration_ * static_cast<float>(count));

	// プレイヤーの座標
	Vector3 playerPos = player_->GetTranslation();
	for (uint32_t i = 0; i < count; ++i) {

		// 発生していなければ
		if (!bulletEmited_[i]) {

			// i番目の時間を計算
			float t = static_cast<float>(i + 1) / static_cast<float>(count);
			if (t <= attackTimer_.t_) {

				// 発生座標
				Vector3 start = launchPositions_[launchIndices_[i]];

				// 弾エフェクト発生
				bulletEffect_->Emit(start);
				// 発生位置、目標座標を設定
				std::vector<Vector3> keys = { start, playerPos };
				bulletEffect_->SetKeyframePath(bulletParticleNodeKey_, keys);
				// 発生済み
				bulletEmited_[i] = true;
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
	SetLaunchPositions(bossEnemy, editingPhase_);
	// 発生順序のインデックスを設定する
	SetLeftToRightIndices(bossEnemy);
	// 発生済みフラグをリセット
	launchEmited_.assign(phaseBulletCounts_[editingPhase_], false);
}

void BossEnemyProjectileAttackState::BeginAttackPhase(BossEnemy& bossEnemy) {

	// 発生済みフラグをリセット
	bulletEmited_.assign(phaseBulletCounts_[editingPhase_], false);
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

void BossEnemyProjectileAttackState::UpdateAlways(BossEnemy& bossEnemy) {

	// エフェクトの更新
	preEffect_->Update();
	launchEffect_->Update();
	bulletEffect_->Update();
}

void BossEnemyProjectileAttackState::Exit(BossEnemy& bossEnemy) {

	// リセット
	canExit_ = false;
	launchTimer_.Reset();
	attackTimer_.Reset();
}

void BossEnemyProjectileAttackState::ImGui(const BossEnemy& bossEnemy) {

	ImGui::Text("currentState: %s", EnumAdapter<State>::ToString(currentState_));

	ImGui::DragInt("editingPhase", &editingPhase_, 1, 0, static_cast<uint32_t>(phaseBulletCounts_.size() - 1));
	ImGui::DragFloat("rotationLerpRate", &rotationLerpRate_, 0.01f);

	ImGui::SeparatorText("Launch");

	ImGui::DragFloat("launchTopPosY", &launchTopPosY_, 0.01f);
	ImGui::DragFloat3("launchOffsetPos", &launchOffsetPos_.x, 0.01f);

	launchTimer_.ImGui("LaunchTimer");

	ImGui::SeparatorText("Attack");

	ImGui::DragFloat("bulletAttackDuration", &bulletAttackDuration_, 0.01f);

	// 座標の更新
	SetLaunchPositions(bossEnemy, editingPhase_);

	ImGui::SeparatorText("Debug");

	int index = 0;
	for (const auto& emited : bulletEmited_) {

		ImGui::Text("bulletEmited[%d]: %s", index++, emited ? "true" : "false");
	}
	// 現在発生させる位置のデバッグ表示
	for (const auto& pos : launchPositions_) {

		LineRenderer::GetInstance()->DrawSphere(6, 1.0f, pos, Color::Cyan());
	}
}

void BossEnemyProjectileAttackState::ApplyJson(const Json& data) {

	launchTimer_.FromJson(data.value("LaunchTimer", Json()));
	bulletAttackDuration_ = data.value("bulletAttackDuration", 1.0f);
	rotationLerpRate_ = data.value("rotationLerpRate_", 1.0f);
	launchTopPosY_ = data.value("launchTopPosY", 4.0f);
	launchOffsetPos_ = Vector3::FromJson(data.value("launchOffsetPos", Json()));
}

void BossEnemyProjectileAttackState::SaveJson(Json& data) {

	launchTimer_.ToJson(data["LaunchTimer"]);
	data["bulletAttackDuration"] = bulletAttackDuration_;
	data["rotationLerpRate_"] = rotationLerpRate_;
	data["launchTopPosY"] = launchTopPosY_;
	data["launchOffsetPos"] = launchOffsetPos_.ToJson();
}