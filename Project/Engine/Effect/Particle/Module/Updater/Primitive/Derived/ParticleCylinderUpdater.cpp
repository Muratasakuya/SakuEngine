#include "ParticleCylinderUpdater.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Helper/Algorithm.h>

//============================================================================
//	ParticleCylinderUpdater classMethods
//============================================================================

void ParticleCylinderUpdater::Init() {

	// 初期化値
	start_.Init();
	target_.Init();
}

void ParticleCylinderUpdater::Update(CPUParticle::ParticleData& particle, EasingType easingType) {

	particle.primitive.cylinder.divide = Algorithm::LerpInt(start_.divide,
		target_.divide, EasedValue(easingType, particle.progress));

	particle.primitive.cylinder.topRadius = std::lerp(start_.topRadius,
		target_.topRadius, EasedValue(easingType, particle.progress));

	particle.primitive.cylinder.bottomRadius = std::lerp(start_.bottomRadius,
		target_.bottomRadius, EasedValue(easingType, particle.progress));

	particle.primitive.cylinder.height = std::lerp(start_.height,
		target_.height, EasedValue(easingType, particle.progress));
}

void ParticleCylinderUpdater::ImGui() {

	ImGui::DragInt("startDivide", &start_.divide, 1, 3, 32);
	ImGui::DragInt("targetDivide", &target_.divide, 1, 3, 32);

	ImGui::DragFloat("startTopRadius", &start_.topRadius, 0.01f);
	ImGui::DragFloat("targetTopRadius", &target_.topRadius, 0.01f);

	ImGui::DragFloat("startTopBottomRadius", &start_.bottomRadius, 0.01f);
	ImGui::DragFloat("targetBottomRadius", &target_.bottomRadius, 0.01f);

	ImGui::DragFloat("startHeight", &start_.height, 0.01f);
	ImGui::DragFloat("targetHeight", &target_.height, 0.01f);
}

void ParticleCylinderUpdater::FromJson(const Json& data) {

	const auto& cylinderData = data["cylinder"];
	start_.divide = cylinderData.value("startDivide", start_.divide);
	target_.divide = cylinderData.value("targetDivide", target_.divide);
	start_.topRadius = cylinderData.value("startTopRadius", start_.topRadius);
	target_.topRadius = cylinderData.value("targetTopRadius", target_.topRadius);
	start_.bottomRadius = cylinderData.value("startBottomRadius", start_.bottomRadius);
	target_.bottomRadius = cylinderData.value("targetBottomRadius", target_.bottomRadius);
	start_.height = cylinderData.value("startHeight", start_.height);
	target_.height = cylinderData.value("targetHeight", target_.height);
}

void ParticleCylinderUpdater::ToJson(Json& data) const {

	data["cylinder"]["startDivide"] = start_.divide;
	data["cylinder"]["targetDivide"] = target_.divide;
	data["cylinder"]["startTopRadius"] = start_.topRadius;
	data["cylinder"]["targetTopRadius"] = target_.topRadius;
	data["cylinder"]["startBottomRadius"] = start_.bottomRadius;
	data["cylinder"]["targetBottomRadius"] = target_.bottomRadius;
	data["cylinder"]["startHeight"] = start_.height;
	data["cylinder"]["targetHeight"] = target_.height;
}