#include "ParticleSpawnConeModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Random/RandomGenerator.h>

//============================================================================
//	ParticleSpawnConeModule classMethods
//============================================================================

void ParticleSpawnConeModule::SetCommand(const ParticleCommand& command) {

	switch (command.id) {
	case ParticleCommandID::SetTranslation: {
		if (const auto& translation = std::get_if<Vector3>(&command.value)) {

			emitter_.translation = *translation;
		}
	}
	case ParticleCommandID::SetRotation: {
		if (const auto& rotation = std::get_if<Vector3>(&command.value)) {

			emitterRotation_ = *rotation;
		}
	}
	}
}

void ParticleSpawnConeModule::Init() {

	// 値の初期値
	ICPUParticleSpawnModule::InitCommonData();
	emitter_.Init();
}

Vector3 ParticleSpawnConeModule::GetFacePoint(float radius, float height) const {

	float angle = RandomGenerator::Generate(0.0f, 2.0f * pi);
	float radiusRandom = RandomGenerator::Generate(0.0f, radius);
	Vector3 point = Vector3(
		radiusRandom * std::cos(angle),
		height,
		radiusRandom * std::sin(angle));

	return point;
}

void ParticleSpawnConeModule::UpdateEmitter() {

	// 回転を更新
	emitter_.rotationMatrix = Matrix4x4::MakeRotateMatrix(emitterRotation_);
}

void ParticleSpawnConeModule::Execute(std::list<CPUParticle::ParticleData>& particles) {

	uint32_t emitCount = emitCount_.GetValue();
	emitter_.rotationMatrix = Matrix4x4::MakeRotateMatrix(emitterRotation_);

	for (uint32_t index = 0; index < emitCount; ++index) {

		CPUParticle::ParticleData particle{};

		// 共通設定
		ICPUParticleSpawnModule::SetCommonData(particle);

		// 上部と下部の面の座標を取得
		Vector3 basePoint = GetFacePoint(emitter_.baseRadius, 0.0f);
		Vector3 topPoint = GetFacePoint(emitter_.topRadius, emitter_.height);
		Vector3 rotatedBasePoint = emitter_.rotationMatrix.TransformPoint(basePoint) + emitter_.translation;
		Vector3 rotatedTopPoint = emitter_.rotationMatrix.TransformPoint(topPoint) + emitter_.translation;

		// 速度、発生位置
		Vector3 direction = (rotatedTopPoint - rotatedBasePoint).Normalize();
		particle.velocity = direction * moveSpeed_.GetValue();
		particle.transform.translation = rotatedBasePoint;

		// 発生した瞬間の座標を記録
		particle.spawnTranlation = particle.transform.translation;

		// 追加
		particles.push_back(particle);
	}
}

void ParticleSpawnConeModule::ImGui() {

	ImGui::DragFloat3("rotation", &emitterRotation_.x, 0.01f);
	ImGui::DragFloat("baseRadius", &emitter_.baseRadius, 0.05f);
	ImGui::DragFloat("topRadius", &emitter_.topRadius, 0.05f);
	ImGui::DragFloat("height", &emitter_.height, 0.05f);
	ImGui::DragFloat3("translation", &emitter_.translation.x, 0.05f);
}

void ParticleSpawnConeModule::DrawEmitter() {

	Vector3 parentTranslation{};
	// 親の座標
	if (parentTransform_) {

		parentTranslation = parentTransform_->matrix.world.GetTranslationValue();
	}

	LineRenderer::GetInstance()->DrawCone(
		8, emitter_.baseRadius, emitter_.topRadius, emitter_.height,
		parentTranslation + emitter_.translation, emitter_.rotationMatrix, emitterLineColor_);
}

Json ParticleSpawnConeModule::ToJson() {

	Json data;

	// 共通設定
	ICPUParticleSpawnModule::ToCommonJson(data);

	data["emitterRotation"] = emitterRotation_.ToJson();

	data["baseRadius"] = emitter_.baseRadius;
	data["topRadius"] = emitter_.topRadius;
	data["height"] = emitter_.height;
	data["translation"] = emitter_.translation.ToJson();

	return data;
}

void ParticleSpawnConeModule::FromJson(const Json& data) {

	// 共通設定
	ICPUParticleSpawnModule::FromCommonJson(data);

	emitterRotation_ = Vector3::FromJson(data["emitterRotation"]);

	emitter_.baseRadius = data.value("baseRadius", 1.0f);
	emitter_.topRadius = data.value("topRadius", 0.5f);
	emitter_.height = data.value("height", 1.0f);
	emitter_.translation = Vector3::FromJson(data["translation"]);
}