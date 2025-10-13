#include "ParticleSpawnHemisphereModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Random/RandomGenerator.h>

//============================================================================
//	ParticleSpawnHemisphereModule classMethods
//============================================================================

bool ParticleSpawnHemisphereModule::SetCommand(const ParticleCommand& command) {

	switch (command.id) {
	case ParticleCommandID::SetTranslation: {
		if (const auto& translation = std::get_if<Vector3>(&command.value)) {

			emitter_.translation = *translation;
			return true;
		}
		return false;
	}
	case ParticleCommandID::SetRotation: {
		if (const auto& rotation = std::get_if<Vector3>(&command.value)) {

			emitterRotation_ = *rotation;
			return true;
		}
		return false;
	}
	}
	return false;
}

void ParticleSpawnHemisphereModule::Init() {

	// 値の初期値
	ICPUParticleSpawnModule::InitCommonData();
	emitter_.Init();
}

Vector3 ParticleSpawnHemisphereModule::GetRandomDirection() const {

	float phi = RandomGenerator::Generate(0.0f, pi * 2.0f);
	float z = RandomGenerator::Generate(-1.0f, 1.0f);
	float sqrtOneMinusZ2 = sqrt(1.0f - z * z);
	Vector3 direction = Vector3(sqrtOneMinusZ2 * cos(phi), sqrtOneMinusZ2 * sin(phi), z);

	// 半球方向限定
	if (direction.y < 0.0f) {
		direction.y = -direction.y;
	}

	return Vector3::Normalize(direction);
}

void ParticleSpawnHemisphereModule::UpdateEmitter() {

	// 回転を更新
	emitter_.rotationMatrix = Matrix4x4::MakeRotateMatrix(emitterRotation_);
}

void ParticleSpawnHemisphereModule::Execute(std::list<CPUParticle::ParticleData>& particles) {

	uint32_t emitCount = emitCount_.GetValue();
	emitter_.rotationMatrix = Matrix4x4::MakeRotateMatrix(emitterRotation_);
	for (uint32_t index = 0; index < emitCount; ++index) {

		CPUParticle::ParticleData particle{};

		// 共通設定
		ICPUParticleSpawnModule::SetCommonData(particle);

		// 速度、発生位置
		Vector3 rotatedDirection = emitter_.rotationMatrix.TransformPoint(GetRandomDirection());
		particle.velocity = rotatedDirection * moveSpeed_.GetValue();
		particle.transform.translation = emitter_.translation + rotatedDirection * emitter_.radius;

		// 発生した瞬間の座標を記録
		particle.spawnTranlation = particle.transform.translation;

		// 追加
		particles.push_back(particle);
	}
}

void ParticleSpawnHemisphereModule::ImGui() {

	ImGui::DragFloat3("rotation", &emitterRotation_.x, 0.01f);
	ImGui::DragFloat("size", &emitter_.radius, 0.05f);
	ImGui::DragFloat3("translation", &emitter_.translation.x, 0.05f);
}

void ParticleSpawnHemisphereModule::DrawEmitter() {

	Vector3 parentTranslation{};
	// 親の座標
	if (parentTransform_) {

		parentTranslation = parentTransform_->matrix.world.GetTranslationValue();
	}

	LineRenderer::GetInstance()->DrawHemisphere(8, emitter_.radius,
		parentTranslation + emitter_.translation, emitter_.rotationMatrix, emitterLineColor_);
}

Json ParticleSpawnHemisphereModule::ToJson() {

	Json data;

	// 共通設定
	ICPUParticleSpawnModule::ToCommonJson(data);

	data["emitterRotation"] = emitterRotation_.ToJson();

	data["radius"] = emitter_.radius;
	data["translation"] = emitter_.translation.ToJson();

	return data;
}

void ParticleSpawnHemisphereModule::FromJson(const Json& data) {

	// 共通設定
	ICPUParticleSpawnModule::FromCommonJson(data);

	emitterRotation_.FromJson(data["emitterRotation"]);

	emitter_.radius = data.value("radius", 1.0f);
	emitter_.translation.FromJson(data["translation"]);
}