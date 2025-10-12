#include "ParticleUpdateAlphaReferenceModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	ParticleUpdateAlphaReferenceModule classMethods
//============================================================================

void ParticleUpdateAlphaReferenceModule::Init() {

	// 初期値の設定
	useNoiseTexture_ = false;
	color_.start = 0.5f;
	color_.target = 0.5f;

	noise_.start = 0.0f;
	noise_.target = 0.5f;
}

void ParticleUpdateAlphaReferenceModule::Execute(
	CPUParticle::ParticleData& particle, [[maybe_unused]] float deltaTime) {

	// 色
	particle.material.alphaReference = std::lerp(
		color_.start, color_.target, EasedValue(easing_, particle.progress));

	// 使用しない場合は処理しない
	particle.textureInfo.useNoiseTexture = static_cast<int32_t>(useNoiseTexture_);
	if (!useNoiseTexture_) {
		return;
	}

	// ノイズ
	particle.material.noiseAlphaReference = std::lerp(
		noise_.start, noise_.target, EasedValue(easing_, particle.progress));
}

void ParticleUpdateAlphaReferenceModule::ImGui() {

	ImGui::Checkbox("useNoiseTexture", &useNoiseTexture_);

	ImGui::DragFloat("startColorReference", &color_.start, 0.01f);
	ImGui::DragFloat("targetColorReference", &color_.target, 0.01f);

	if (!useNoiseTexture_) {
		return;
	}

	ImGui::DragFloat("starttNoiseReference", &noise_.start, 0.01f);
	ImGui::DragFloat("targetNoiseReference", &noise_.target, 0.01f);

	Easing::SelectEasingType(easing_, GetName());
}

Json ParticleUpdateAlphaReferenceModule::ToJson() {

	Json data;

	data["useNoiseTexture"] = useNoiseTexture_;

	// 棄却値
	data["color"]["start"] = color_.start;
	data["color"]["target"] = color_.target;

	data["noise"]["start"] = noise_.start;
	data["noise"]["target"] = noise_.target;

	data["easingType"] = EnumAdapter<EasingType>::ToString(easing_);

	return data;
}

void ParticleUpdateAlphaReferenceModule::FromJson(const Json& data) {

	useNoiseTexture_ = data.value("useNoiseTexture", false);

	// イージング
	const auto& easingType = EnumAdapter<EasingType>::FromString(data.value("easingType", ""));
	easing_ = easingType.value();

	// 色
	const auto& colorData = data["color"];
	color_.start = colorData.value("start", 0.5f);
	color_.target = colorData.value("target", 0.5f);

	// ノイズ
	const auto& noiseData = data["noise"];
	noise_.start = noiseData.value("start", 0.0f);
	noise_.target = noiseData.value("target", 0.5f);
}