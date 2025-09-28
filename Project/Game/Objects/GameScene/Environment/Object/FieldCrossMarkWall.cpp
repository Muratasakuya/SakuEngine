#include "FieldCrossMarkWall.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	FieldCrossMarkWall classMethods
//============================================================================

void FieldCrossMarkWall::DerivedInit() {

	// json適応
	ApplyJson();
}

void FieldCrossMarkWall::Update() {

	const float deltaTime = GameTimer::GetDeltaTime();

	switch (state_) {
	case State::Idle:
		break;
	case State::Entering: {

		lerpTimer_ += deltaTime;
		const float t = std::min(lerpTimer_ / lerpTime_, 1.0f);
		currentColor_ = Color::Lerp(startColor_, targetColor_, t);
		currentEmissive_ = std::lerp(startEmissive_, 1.0f, t);
		if (1.0f <= t) {

			state_ = State::Staying;
			blinkTimer_ = 0.0f;
		}
		break;
	}
	case State::Staying: {

		blinkTimer_ += deltaTime;
		const float t = std::fmod(blinkTimer_, blinkingSpacing_) / blinkingSpacing_;
		currentEmissive_ = (t < 0.5f) ? (t * 2.0f) : (1.0f - (t - 0.5f) * 2.0f);
		break;
	}
	case State::Exiting: {

		lerpTimer_ += deltaTime;
		const float t = (std::min)(lerpTimer_ / lerpTime_, 1.0f);
		currentColor_ = Color::Lerp(startColor_, initColor_, t);
		currentEmissive_ = std::lerp(startEmissive_, 0.0f, t);
		if (1.0f <= t) {

			state_ = State::Idle;
		}
		break;
	}
	}

	materials_->front().emissiveIntensity = currentEmissive_;
	materials_->front().color = currentColor_;

	// collision更新
	Collider::UpdateAllBodies(*transform_);
}

void FieldCrossMarkWall::OnCollisionEnter([[maybe_unused]] const CollisionBody* collisionBody) {

	// 衝突中の場合は処理しない
	if (state_ == State::Staying || state_ == State::Entering) {
		return;
	}

	lerpTimer_ = 0.0f;
	state_ = State::Entering;
	startColor_ = currentColor_;
	startEmissive_ = currentEmissive_;
}

void FieldCrossMarkWall::OnCollisionStay([[maybe_unused]] const CollisionBody* collisionBody) {
}

void FieldCrossMarkWall::OnCollisionExit([[maybe_unused]] const CollisionBody* collisionBody) {

	if (state_ == State::Exiting || state_ == State::Idle) {
		return;
	}

	lerpTimer_ = 0.0f;
	state_ = State::Exiting;
	startColor_ = currentColor_;
	startEmissive_ = currentEmissive_;
}

void FieldCrossMarkWall::DerivedImGui() {

	if (ImGui::Button("Save")) {

		SaveJson();
	}

	ImGui::DragFloat("lerpTime", &lerpTime_, 0.01f);
	ImGui::DragFloat("blinkingSpacing", &blinkingSpacing_, 0.01f);

	ImGui::ColorEdit4("initColor", &initColor_.r);
	ImGui::ColorEdit4("targetColor", &targetColor_.r);
}

void FieldCrossMarkWall::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Level/ObjectData/fieldCrossMarkWallParam.json", data)) {
		return;
	}

	lerpTime_ = data["lerpTime_"];
	blinkingSpacing_ = data["blinkingSpacing_"];
	initColor_ = initColor_.FromJson(data["initColor_"]);
	currentColor_ = initColor_;
	targetColor_ = targetColor_.FromJson(data["targetColor_"]);
}

void FieldCrossMarkWall::SaveJson() {

	Json data;

	data["lerpTime_"] = lerpTime_;
	data["blinkingSpacing_"] = blinkingSpacing_;
	data["initColor_"] = initColor_.ToJson();
	data["targetColor_"] = targetColor_.ToJson();

	JsonAdapter::Save("Level/ObjectData/fieldCrossMarkWallParam.json", data);
}