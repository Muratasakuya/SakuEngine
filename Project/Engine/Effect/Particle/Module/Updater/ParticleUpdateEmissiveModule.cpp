#include "ParticleUpdateEmissiveModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	ParticleUpdateEmissiveModule classMethods
//============================================================================

void ParticleUpdateEmissiveModule::Init() {

	// 初期値の設定
	intencity_.start = 1.0f;
	intencity_.target = 1.0f;

	color_.start = Vector3::AnyInit(1.0f);
	color_.target = Vector3::AnyInit(1.0f);
}

void ParticleUpdateEmissiveModule::Execute(
	CPUParticle::ParticleData& particle, [[maybe_unused]] float deltaTime) {

	// 発光度
	particle.material.emissiveIntecity = std::lerp(
		intencity_.start, intencity_.target, EasedValue(easingType_, particle.progress));
	// 色
	particle.material.emissionColor = Vector3::Lerp(
		color_.start, color_.target, EasedValue(easingType_, particle.progress));
}

void ParticleUpdateEmissiveModule::ImGui() {

	ImGui::DragFloat("startIntencity", &intencity_.start, 0.01f);
	ImGui::DragFloat("targetIntencity", &intencity_.target, 0.01f);

	ImGui::ColorEdit3("startColor", &color_.start.x);
	ImGui::ColorEdit3("targetColor", &color_.target.x);

	Easing::SelectEasingType(easingType_, GetName());
}

Json ParticleUpdateEmissiveModule::ToJson() {

	Json data;

	// 発光度
	data["intencity"]["start"] = intencity_.start;
	data["intencity"]["target"] = intencity_.target;

	// 発光色
	data["color"]["start"] = color_.start.ToJson();
	data["color"]["target"] = color_.target.ToJson();

	data["easingType"] = EnumAdapter<EasingType>::ToString(easingType_);

	return data;
}

void ParticleUpdateEmissiveModule::FromJson(const Json& data) {

	const auto& easingType = EnumAdapter<EasingType>::FromString(data.value("easingType", ""));
	easingType_ = easingType.value();

	// 発光度
	const auto& intData = data["intencity"];
	intencity_.start = intData.value("start", 1.0f);
	intencity_.target = intData.value("target", 1.0f);

	// 発光色
	const auto& colData = data["color"];
	color_.start = color_.start.FromJson(colData["start"]);
	color_.target = color_.target.FromJson(colData["target"]);
}