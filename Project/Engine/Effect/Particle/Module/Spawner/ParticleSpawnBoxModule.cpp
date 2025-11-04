#include "ParticleSpawnBoxModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Random/RandomGenerator.h>

//============================================================================
//	ParticleSpawnBoxModule classMethods
//============================================================================

void ParticleSpawnBoxModule::SetCommand(const ParticleCommand& command) {

	switch (command.id) {
	case ParticleCommandID::SetTranslation: {
		if (const auto& translation = std::get_if<Vector3>(&command.value)) {

			emitter_.translation = *translation;
		}
		break;
	}
	case ParticleCommandID::SetRotation: {
		if (const auto& rotation = std::get_if<Vector3>(&command.value)) {

			emitterRotation_ = *rotation;
		}
		break;
	}
	}
}

void ParticleSpawnBoxModule::Init() {

	// 値の初期値
	ICPUParticleSpawnModule::InitCommonData();
	emitter_.Init();
}

Vector3 ParticleSpawnBoxModule::GetRandomPoint() const {

	Vector3 halfSize = emitter_.size * 0.5f;
	return RandomGenerator::Generate(Vector3(-halfSize.x, -halfSize.y, -halfSize.z), halfSize);
}

void ParticleSpawnBoxModule::UpdateEmitter() {

	// 回転を更新
	emitter_.rotationMatrix = Matrix4x4::MakeRotateMatrix(emitterRotation_);
}

void ParticleSpawnBoxModule::Execute(std::list<CPUParticle::ParticleData>& particles) {

	uint32_t emitCount = emitCount_.GetValue();
	// +Z方向に飛ばす
	Vector3 forward = Vector3::Normalize(Vector3::TransferNormal(Vector3(0.0f, 0.0f, 1.0f), emitter_.rotationMatrix));
	for (uint32_t index = 0; index < emitCount; ++index) {

		CPUParticle::ParticleData particle{};

		// 共通設定
		ICPUParticleSpawnModule::SetCommonData(particle);

		// 速度、発生位置
		particle.velocity = forward * moveSpeed_.GetValue();
		particle.transform.translation = emitter_.rotationMatrix.TransformPoint(GetRandomPoint()) + emitter_.translation;

		// 発生した瞬間の座標を記録
		particle.spawnTranlation = particle.transform.translation;

		// 追加
		particles.push_back(particle);
	}
}

void ParticleSpawnBoxModule::ImGui() {

	ImGui::DragFloat3("rotation", &emitterRotation_.x, 0.01f);
	ImGui::DragFloat3("size", &emitter_.size.x, 0.05f);
	ImGui::DragFloat3("translation", &emitter_.translation.x, 0.05f);
}

void ParticleSpawnBoxModule::DrawEmitter() {

	Vector3 parentTranslation{};
	// 親の座標
	if (parentTransform_) {

		parentTranslation = parentTransform_->matrix.world.GetTranslationValue();
	}

	LineRenderer::GetInstance()->DrawOBB(parentTranslation + emitter_.translation,
		emitter_.size, emitter_.rotationMatrix, emitterLineColor_);
}

Json ParticleSpawnBoxModule::ToJson() {

	Json data;

	// 共通設定
	ICPUParticleSpawnModule::ToCommonJson(data);

	data["emitterRotation"] = emitterRotation_.ToJson();

	data["size"] = emitter_.size.ToJson();
	data["translation"] = emitter_.translation.ToJson();

	return data;
}

void ParticleSpawnBoxModule::FromJson(const Json& data) {

	// 共通設定
	ICPUParticleSpawnModule::FromCommonJson(data);

	emitterRotation_ = Vector3::FromJson(data["emitterRotation"]);

	emitter_.size = Vector3::FromJson(data["size"]);
	emitter_.translation = Vector3::FromJson(data["translation"]);
}