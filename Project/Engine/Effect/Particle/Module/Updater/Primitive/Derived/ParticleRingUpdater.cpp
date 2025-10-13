#include "ParticleRingUpdater.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/Algorithm.h>

//============================================================================
//	ParticleRingUpdater classMethods
//============================================================================

void ParticleRingUpdater::Init() {

	// 初期化値
	start_.Init();
	target_.Init();
}

void ParticleRingUpdater::Update(CPUParticle::ParticleData& particle, EasingType easingType) {

	particle.primitive.ring.divide = Algorithm::LerpInt(start_.divide,
		target_.divide, EasedValue(easingType, particle.progress));

	particle.primitive.ring.outerRadius = std::lerp(start_.outerRadius,
		target_.outerRadius, EasedValue(easingType, particle.progress));

	particle.primitive.ring.innerRadius = std::lerp(start_.innerRadius,
		target_.innerRadius, EasedValue(easingType, particle.progress));
}

void ParticleRingUpdater::ImGui() {

	ImGui::DragInt("startDivide", &start_.divide, 1, 3, 32);
	ImGui::DragInt("targetDivide", &target_.divide, 1, 3, 32);

	ImGui::DragFloat("startOuterRadius", &start_.outerRadius, 0.01f);
	ImGui::DragFloat("targetOuterRadius", &target_.outerRadius, 0.01f);

	ImGui::DragFloat("startInnerRadius", &start_.innerRadius, 0.01f);
	ImGui::DragFloat("targetInnerRadius", &target_.innerRadius, 0.01f);
}

void ParticleRingUpdater::FromJson(const Json& data) {

	const auto& ringData = data["ring"];
	start_.divide = ringData.value("startDivide", start_.divide);
	target_.divide = ringData.value("targetDivide", target_.divide);
	start_.outerRadius = ringData.value("startOuterRadius", start_.outerRadius);
	target_.outerRadius = ringData.value("targetOuterRadius", target_.outerRadius);
	start_.innerRadius = ringData.value("startInnerRadius", start_.innerRadius);
	target_.innerRadius = ringData.value("targetInnerRadius", target_.innerRadius);
}

void ParticleRingUpdater::ToJson(Json& data) const {

	data["ring"]["startDivide"] = start_.divide;
	data["ring"]["targetDivide"] = target_.divide;
	data["ring"]["startOuterRadius"] = start_.outerRadius;
	data["ring"]["targetOuterRadius"] = target_.outerRadius;
	data["ring"]["startInnerRadius"] = start_.innerRadius;
	data["ring"]["targetInnerRadius"] = target_.innerRadius;
}