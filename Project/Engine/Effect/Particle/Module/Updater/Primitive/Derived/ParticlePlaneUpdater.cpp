#include "ParticlePlaneUpdater.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	ParticlePlaneUpdater classMethods
//============================================================================

void ParticlePlaneUpdater::Init() {

	// 初期化値
	start_.Init();
	target_.Init();
	planeType_ = ParticlePlaneType::XY;
}

void ParticlePlaneUpdater::Update(CPUParticle::ParticleData& particle, EasingType easingType) {

	// 形状モードを更新
	particle.primitive.plane.mode = start_.mode;
	particle.primitive.plane.mode = target_.mode;

	particle.primitive.plane.size = Vector2::Lerp(start_.size,
		target_.size, EasedValue(easingType, particle.progress));

	particle.primitive.plane.pivot = Vector2::Lerp(start_.pivot,
		target_.pivot, EasedValue(easingType, particle.progress));
}

void ParticlePlaneUpdater::ImGui() {

	ImGui::DragFloat2("startSize", &start_.size.x, 0.01f);
	ImGui::DragFloat2("targetSize", &target_.size.x, 0.01f);

	ImGui::DragFloat2("startPivot", &start_.pivot.x, 0.01f);
	ImGui::DragFloat2("targetPivot", &target_.pivot.x, 0.01f);

	if (EnumAdapter<ParticlePlaneType>::Combo("planeType", &planeType_)) {

		start_.mode = static_cast<uint32_t>(planeType_);
		target_.mode = static_cast<uint32_t>(planeType_);
	}
}

void ParticlePlaneUpdater::FromJson(const Json& data) {

	const auto& planeData = data["plane"];
	start_.size = start_.size.FromJson(planeData["startSize"]);
	target_.size = target_.size.FromJson(planeData["targetSize"]);
	start_.pivot = start_.pivot.FromJson(planeData["startPivot"]);
	target_.pivot = target_.pivot.FromJson(planeData["targetPivot"]);

	const auto& planeType = EnumAdapter<ParticlePlaneType>::FromString(data["plane"]["mode"]);
	start_.mode = static_cast<uint32_t>(planeType.value());
	target_.mode = static_cast<uint32_t>(planeType.value());
	planeType_ = planeType.value();
}

void ParticlePlaneUpdater::ToJson(Json& data) const {

	data["plane"]["startSize"] = start_.size.ToJson();
	data["plane"]["targetSize"] = target_.size.ToJson();
	data["plane"]["startPivot"] = start_.pivot.ToJson();
	data["plane"]["targetPivot"] = target_.pivot.ToJson();
	data["plane"]["mode"] = EnumAdapter<ParticlePlaneType>::ToString(planeType_);
}