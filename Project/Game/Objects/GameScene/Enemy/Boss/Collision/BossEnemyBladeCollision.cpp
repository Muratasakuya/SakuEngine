#include "BossEnemyBladeCollision.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Json/JsonAdapter.h>

// imgui
#include <imgui.h>

//============================================================================
//	BossEnemyBladeCollision classMethods
//============================================================================

void BossEnemyBladeCollision::Init(const std::string& typeName) {

	typeName_ = typeName;
	// ファイルの名前を設定
	fileName_ = "Enemy/Boss/" + typeName_ + ".json";

	// 形状初期化
	bodies_.emplace_back(Collider::AddCollider(CollisionShape::OBB().Default()));
	bodyOffsets_.emplace_back(CollisionShape::OBB().Default());

	// タイプ設定
	bodies_.front()->SetType(ColliderType::Type_BossBlade);
	bodies_.front()->SetTargetType(ColliderType::Type_Player);

	// transform初期化
	transform_.Init();

	isEmit_ = false;
	lifeTimer_ = 0.0f;

	// json適応
	ApplyJson();
}

void BossEnemyBladeCollision::EmitEffect(const Vector3& emitPos, const Vector3& velocity) {

	// 発生させる
	isEmit_ = true;
	lifeTimer_ = 0.0f;

	// 速度を設定
	velocity_ = velocity;

	// 座標を設定
	transform_.translation = emitPos;

	// 進行方向に回転を設定する
	Vector3 direction = Vector3(velocity_.x, 0.0f, velocity_.z).Normalize();
	transform_.rotation = Quaternion::LookRotation(direction, Vector3(0.0f, 1.0f, 0.0f));

	// transformを元に戻す
	transform_.scale = scale_;
	transform_.translation.y = 0.0f;
}

void BossEnemyBladeCollision::Update() {

	// 発生していない間は更新しない
	if (!isEmit_) {
		return;
	}

	// 時間経過の更新処理
	UpdateLifeTime();

	// 移動更新
	UpdateMove();

	// 衝突更新
	Collider::UpdateAllBodies(transform_);
}

void BossEnemyBladeCollision::UpdateMove() {

	// 速度を加算していく
	transform_.translation += velocity_ * GameTimer::GetScaledDeltaTime();

	// 行列更新
	transform_.UpdateMatrix();
}

void BossEnemyBladeCollision::UpdateLifeTime() {

	// 時間を進める
	lifeTimer_ += GameTimer::GetScaledDeltaTime();

	// 時間が経過しきったら衝突しないように消す
	if (lifeTime_ < lifeTimer_) {

		isEmit_ = false;
		lifeTimer_ = 0.0f;
		transform_.scale = Vector3::AnyInit(0.0f);
		transform_.translation.y = outsideTranslationY_;
	}
}

void BossEnemyBladeCollision::ImGui() {

	if (ImGui::Button(("Save Json##" + typeName_).c_str())) {

		SaveJson();
	}
	if (ImGui::Button(("Apply Json##" + typeName_).c_str())) {

		ApplyJson();
	}

	ImGui::Text(std::format("isEmit: {}", isEmit_).c_str());
	ImGui::Text(std::format("lifeTimer: {}", lifeTimer_).c_str());

	// scaleを設定する
	if (ImGui::DragFloat3(("scale##" + typeName_).c_str(), &scale_.x, 0.1f)) {

		transform_.scale = scale_;
		transform_.UpdateMatrix();

		// 衝突更新
		Collider::UpdateAllBodies(transform_);
	}

	ImGui::DragFloat(("lifeTime##" + typeName_).c_str(), &lifeTime_, 0.01f);
	ImGui::DragFloat(("outsideTranslationY##" + typeName_).c_str(), &outsideTranslationY_, 1.0f);
}

void BossEnemyBladeCollision::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck(fileName_, data)) {
		return;
	}

	lifeTime_ = JsonAdapter::GetValue<float>(data, "lifeTime_");
	outsideTranslationY_ = JsonAdapter::GetValue<float>(data, "outsideTranslationY_");
	scale_ = JsonAdapter::ToObject<Vector3>(data["scale_"]);

	transform_.translation.y = outsideTranslationY_;
	Collider::UpdateAllBodies(transform_);
}

void BossEnemyBladeCollision::SaveJson() {

	Json data;

	data["lifeTime_"] = lifeTime_;
	data["outsideTranslationY_"] = outsideTranslationY_;
	data["scale_"] = JsonAdapter::FromObject<Vector3>(scale_);

	JsonAdapter::Save(fileName_, data);
}