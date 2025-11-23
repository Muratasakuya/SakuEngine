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

	particle.primitive.cylinder.maxAngle = std::lerp(start_.maxAngle,
		target_.maxAngle, EasedValue(easingType, particle.progress));

	particle.primitive.cylinder.height = std::lerp(start_.height,
		target_.height, EasedValue(easingType, particle.progress));

	particle.primitive.cylinder.topColor = Color::Lerp(start_.topColor,
		target_.topColor, EasedValue(easingType, particle.progress));

	particle.primitive.cylinder.bottomColor = Color::Lerp(start_.bottomColor,
		target_.bottomColor, EasedValue(easingType, particle.progress));
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

	ImGui::DragFloat("startMaxAngle", &start_.maxAngle, 0.01f);
	ImGui::DragFloat("targetMaxAngle", &target_.maxAngle, 0.01f);

	ImGui::ColorEdit4("startTopColor", &start_.topColor.r);
	ImGui::ColorEdit4("targetTopColor", &target_.topColor.r);

	ImGui::ColorEdit4("startBottomColor", &start_.bottomColor.r);
	ImGui::ColorEdit4("targetBottomColor", &target_.bottomColor.r);
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
	start_.maxAngle = cylinderData.value("startMaxAngle", start_.maxAngle);
	target_.maxAngle = cylinderData.value("targetMaxAngle", target_.maxAngle);	
	start_.topColor = Color::FromJson(cylinderData.value("startTopColor", Json()));
	target_.topColor = Color::FromJson(cylinderData.value("targetTopColor", Json()));
	start_.bottomColor = Color::FromJson(cylinderData.value("startBottomColor", Json()));
	target_.bottomColor = Color::FromJson(cylinderData.value("targetBottomColor", Json()));
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
	data["cylinder"]["startMaxAngle"] = start_.maxAngle;
	data["cylinder"]["targetMaxAngle"] = target_.maxAngle;
	data["cylinder"]["startTopColor"] = start_.topColor.ToJson();
	data["cylinder"]["targetTopColor"] = target_.topColor.ToJson();
	data["cylinder"]["startBottomColor"] = start_.bottomColor.ToJson();
	data["cylinder"]["targetBottomColor"] = target_.bottomColor.ToJson();
}