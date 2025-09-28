#include "ParticleSpawnModuleUpdater.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

// imgui
#include <imgui.h>

//============================================================================
//	ParticleSpawnModuleUpdater classMethods
//============================================================================

void ParticlePolygonVertexUpdater::Update(float& scale, Vector3& rotation) {

	// 時間経過が終了していれば処理しない
	if (timer_.IsReached()) {

		// 値を固定
		scale = scale_.target;
		rotation = rotation_.target;
		return;
	}

	// 時間経過を進める
	timer_.Update();

	// 値を補間
	scale = std::lerp(scale_.start, scale_.target,
		EasedValue(scale_.easing, timer_.t_));
	rotation = Vector3::Lerp(rotation_.start, rotation_.target,
		EasedValue(rotation_.easing, timer_.t_));
}

void ParticlePolygonVertexUpdater::ImGui() {

	if (ImGui::Button("Reset")) {

		timer_.Reset();
	}

	ImGui::DragFloat("startScale", &scale_.start, 0.1f);
	ImGui::DragFloat("targetScale", &scale_.target, 0.1f);
	Easing::SelectEasingType(scale_.easing, "scale_");

	ImGui::DragFloat3("startRotation", &rotation_.start.x, 0.01f);
	ImGui::DragFloat3("targetRotation", &rotation_.target.x, 0.01f);
	Easing::SelectEasingType(rotation_.easing, "rotation_");

	timer_.ImGui("AnimTimer");
}

void ParticlePolygonVertexUpdater::FromJson(const Json& data) {

	timer_.FromJson(data["AnimTimer"]);

	scale_.start = data.value("startScale", 4.0f);
	scale_.target = data.value("targetScale", 0.0f);

	rotation_.start = Vector3::FromJson(data.value("startRotation", Json()));
	rotation_.target = Vector3::FromJson(data.value("targetRotation", Json()));

	// イージング
	scale_.easing = EnumAdapter<EasingType>::FromString(data["scaleEasing"]).value();
	rotation_.easing = EnumAdapter<EasingType>::FromString(data["rotationEasing"]).value();
}

void ParticlePolygonVertexUpdater::ToJson(Json& data) {

	timer_.ToJson(data["AnimTimer"]);

	data["startScale"] = scale_.start;
	data["targetScale"] = scale_.target;

	data["startRotation"] = rotation_.start.ToJson();
	data["targetRotation"] = rotation_.target.ToJson();

	data["scaleEasing"] = EnumAdapter<EasingType>::ToString(scale_.easing);
	data["rotationEasing"] = EnumAdapter<EasingType>::ToString(rotation_.easing);
}

void ParticlePolygonVertexUpdater::SetOffsetRotation(Vector3 offsetRotation) {

	rotation_.start += offsetRotation;
	rotation_.target += offsetRotation;
}