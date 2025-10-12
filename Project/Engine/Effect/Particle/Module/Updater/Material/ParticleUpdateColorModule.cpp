#include "ParticleUpdateColorModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	ParticleUpdateColorModule classMethods
//============================================================================

void ParticleUpdateColorModule::Init() {

	// 初期化値
	color_.start = Color::White();
	color_.target = Color::White(0.0f);
}

void ParticleUpdateColorModule::Execute(
	CPUParticle::ParticleData& particle, [[maybe_unused]] float deltaTime) {

	// t値取得
	const float lerpT = LoopedT(particle.progress);

	// 色を補間
	particle.material.color = Color::Lerp(color_.start, color_.target,
		EasedValue(easing, lerpT));
}

void ParticleUpdateColorModule::ImGui() {

	ImGui::ColorEdit4("startColor", &color_.start.r);
	ImGui::ColorEdit4("targetColor", &color_.target.r);

	Easing::SelectEasingType(easing, GetName());

	ImGuiLoopParam();
}

Json ParticleUpdateColorModule::ToJson() {

	Json data;

	// ループ
	ParticleLoopableModule::ToLoopJson(data);

	data["color"]["start"] = color_.start.ToJson();
	data["color"]["target"] = color_.target.ToJson();

	data["easingType"] = EnumAdapter<EasingType>::ToString(easing);

	return data;
}

void ParticleUpdateColorModule::FromJson(const Json& data) {

	// ループ
	ParticleLoopableModule::FromLoopJson(data);

	const auto& easingType = EnumAdapter<EasingType>::FromString(data.value("easingType", ""));
	easing = easingType.value();

	const auto& colorData = data["color"];
	color_.start = color_.start.FromJson(colorData["start"]);
	color_.target = color_.target.FromJson(colorData["target"]);
}