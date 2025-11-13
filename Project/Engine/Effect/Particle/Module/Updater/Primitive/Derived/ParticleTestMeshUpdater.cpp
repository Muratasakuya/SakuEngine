#include "ParticleTestMeshUpdater.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	ParticleTestMeshUpdater classMethods
//============================================================================

void ParticleTestMeshUpdater::Init() {

	// 初期化値
	start_.Init();
	target_.Init();
	planeType_ = ParticlePlaneType::XY;
}

void ParticleTestMeshUpdater::Update(CPUParticle::ParticleData& particle, EasingType easingType) {

	// 形状モードを更新
	particle.primitive.testMesh.mode = start_.mode;
	particle.primitive.testMesh.mode = target_.mode;

	particle.primitive.testMesh.size = Vector2::Lerp(start_.size,
		target_.size, EasedValue(easingType, particle.progress));

	particle.primitive.testMesh.pivot = Vector2::Lerp(start_.pivot,
		target_.pivot, EasedValue(easingType, particle.progress));
}

void ParticleTestMeshUpdater::ImGui() {

	ImGui::DragFloat2("startSize", &start_.size.x, 0.01f);
	ImGui::DragFloat2("targetSize", &target_.size.x, 0.01f);

	ImGui::DragFloat2("startPivot", &start_.pivot.x, 0.01f);
	ImGui::DragFloat2("targetPivot", &target_.pivot.x, 0.01f);

	if (EnumAdapter<ParticlePlaneType>::Combo("planeType", &planeType_)) {

		start_.mode = static_cast<uint32_t>(planeType_);
		target_.mode = static_cast<uint32_t>(planeType_);
	}
}

void ParticleTestMeshUpdater::FromJson(const Json& data) {

	if (!data.contains("testMesh")) {
		return;
	}

	const auto& testMeshData = data["testMesh"];
	start_.size = start_.size.FromJson(testMeshData["startSize"]);
	target_.size = target_.size.FromJson(testMeshData["targetSize"]);
	start_.pivot = start_.pivot.FromJson(testMeshData["startPivot"]);
	target_.pivot = target_.pivot.FromJson(testMeshData["targetPivot"]);

	const auto& planeType = EnumAdapter<ParticlePlaneType>::FromString(data["plane"]["mode"]);
	start_.mode = static_cast<uint32_t>(planeType.value());
	target_.mode = static_cast<uint32_t>(planeType.value());
	planeType_ = planeType.value();
}

void ParticleTestMeshUpdater::ToJson(Json& data) const {

	data["testMesh"]["startSize"] = start_.size.ToJson();
	data["testMesh"]["targetSize"] = target_.size.ToJson();
	data["testMesh"]["startPivot"] = start_.pivot.ToJson();
	data["testMesh"]["targetPivot"] = target_.pivot.ToJson();
	data["testMesh"]["mode"] = EnumAdapter<ParticlePlaneType>::ToString(planeType_);
}