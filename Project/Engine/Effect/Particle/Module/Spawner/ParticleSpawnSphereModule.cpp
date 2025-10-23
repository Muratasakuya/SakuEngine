#include "ParticleSpawnSphereModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Random/RandomGenerator.h>

//============================================================================
//	ParticleSpawnSphereModule classMethods
//============================================================================

void ParticleSpawnSphereModule::SetCommand(const ParticleCommand& command) {

	switch (command.id) {
	case ParticleCommandID::SetTranslation: {
		if (const auto& translation = std::get_if<Vector3>(&command.value)) {

			emitter_.translation = *translation;
		}
	}
	}
}

void ParticleSpawnSphereModule::Init() {

	// 値の初期値
	ICPUParticleSpawnModule::InitCommonData();
	emitter_.Init();
}

Vector3 ParticleSpawnSphereModule::GetRandomDirection() const {

	float phi = RandomGenerator::Generate(0.0f, pi * 2.0f);
	float z = RandomGenerator::Generate(-1.0f, 1.0f);
	float sqrtOneMinusZ2 = sqrt(1.0f - z * z);
	Vector3 direction = Vector3(sqrtOneMinusZ2 * cos(phi), sqrtOneMinusZ2 * sin(phi), z);

	return Vector3::Normalize(direction);
}

void ParticleSpawnSphereModule::Execute(std::list<CPUParticle::ParticleData>& particles) {

	uint32_t emitCount = emitCount_.GetValue();
	for (uint32_t index = 0; index < emitCount; ++index) {

		CPUParticle::ParticleData particle{};

		// 共通設定
		ICPUParticleSpawnModule::SetCommonData(particle);

		// 速度、発生位置
		Vector3 direction = GetRandomDirection();
		particle.velocity = direction * moveSpeed_.GetValue();
		particle.transform.translation = emitter_.translation + direction * emitter_.radius;

		// 発生した瞬間の座標を記録
		particle.spawnTranlation = particle.transform.translation;

		// 追加
		particles.push_back(particle);
	}
}

void ParticleSpawnSphereModule::ImGui() {

	ImGui::DragFloat("radius", &emitter_.radius, 0.1f);
	ImGui::DragFloat3("translation", &emitter_.translation.x, 0.05f);
}

void ParticleSpawnSphereModule::DrawEmitter() {

	Vector3 parentTranslation{};
	// 親の座標
	if (parentTransform_) {

		parentTranslation = parentTransform_->matrix.world.GetTranslationValue();
	}
	LineRenderer::GetInstance()->DrawSphere(4, emitter_.radius,
		parentTranslation + emitter_.translation, emitterLineColor_);
}

Json ParticleSpawnSphereModule::ToJson() {

	Json data;

	// 共通設定
	ICPUParticleSpawnModule::ToCommonJson(data);

	data["radius"] = emitter_.radius;
	data["translation"] = emitter_.translation.ToJson();

	return data;
}

void ParticleSpawnSphereModule::FromJson(const Json& data) {

	// 共通設定
	ICPUParticleSpawnModule::FromCommonJson(data);

	emitter_.radius = data.value("radius", 1.0f);
	emitter_.translation = Vector3::FromJson(data["translation"]);
}