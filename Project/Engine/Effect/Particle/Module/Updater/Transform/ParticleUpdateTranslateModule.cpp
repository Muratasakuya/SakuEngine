#include "ParticleUpdateTranslateModule.h"

//============================================================================
//	ParticleUpdateTranslateModule classMethods
//============================================================================

void ParticleUpdateTranslateModule::SetCommand(const ParticleCommand& command) {

	switch (command.id) {
	case ParticleCommandID::SetTranslation: {
		if (const auto& translation = std::get_if<Vector3>(&command.value)) {

			translation_ = *translation;
		}
	}
	}
}

void ParticleUpdateTranslateModule::Init() {

	// 初期化値
	dragValue_ = 0.01f;
	translation_ = Vector3(0.0, 4.0f, 0.0f);
}

void ParticleUpdateTranslateModule::Execute(
	CPUParticle::ParticleData& particle, [[maybe_unused]] float deltaTime) {

	// 座標を渡す
	particle.transform.translation = translation_;
}

void ParticleUpdateTranslateModule::ImGui() {

	ImGui::DragFloat("dragValue", &dragValue_, 0.001f, 0.001f, 16.0f);
	ImGui::DragFloat3("translation", &translation_.x, dragValue_);
}

Json ParticleUpdateTranslateModule::ToJson() {

	Json data;

	data["translation_"] = translation_.ToJson();
	data["dragValue_"] = dragValue_;

	return data;
}

void ParticleUpdateTranslateModule::FromJson(const Json& data) {

	translation_ = Vector3::FromJson(data.value("translation_", Json()));
	dragValue_ = data.value("dragValue_", 0.01f);
}