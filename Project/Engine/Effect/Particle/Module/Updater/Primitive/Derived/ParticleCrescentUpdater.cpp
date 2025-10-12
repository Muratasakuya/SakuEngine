#include "ParticleCrescentUpdater.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/Algorithm.h>

//============================================================================
//	ParticleCrescentUpdater classMethods
//============================================================================

void ParticleCrescentUpdater::Init() {

	// 初期化値
	start_.Init();
	target_.Init();
}

void ParticleCrescentUpdater::Update(CPUParticle::ParticleData& particle, EasingType easingType) {

	particle.primitive.crescent.uvMode = start_.uvMode;
	particle.primitive.crescent.uvMode = target_.uvMode;

	particle.primitive.crescent.divide = Algorithm::LerpInt(start_.divide,
		target_.divide, EasedValue(easingType, particle.progress));

	particle.primitive.crescent.outerRadius = std::lerp(start_.outerRadius,
		target_.outerRadius, EasedValue(easingType, particle.progress));

	particle.primitive.crescent.innerRadius = std::lerp(start_.innerRadius,
		target_.innerRadius, EasedValue(easingType, particle.progress));

	particle.primitive.crescent.startAngle = std::lerp(start_.startAngle,
		target_.startAngle, EasedValue(easingType, particle.progress));

	particle.primitive.crescent.endAngle = std::lerp(start_.endAngle,
		target_.endAngle, EasedValue(easingType, particle.progress));

	particle.primitive.crescent.lattice = std::lerp(start_.lattice,
		target_.lattice, EasedValue(easingType, particle.progress));

	particle.primitive.crescent.thickness = std::lerp(start_.thickness,
		target_.thickness, EasedValue(easingType, particle.progress));

	particle.primitive.crescent.pivot = Vector2::Lerp(start_.pivot,
		target_.pivot, EasedValue(easingType, particle.progress));

	particle.primitive.crescent.outerColor = Color::Lerp(start_.outerColor,
		target_.outerColor, EasedValue(easingType, particle.progress));

	particle.primitive.crescent.innerColor = Color::Lerp(start_.innerColor,
		target_.innerColor, EasedValue(easingType, particle.progress));
}

void ParticleCrescentUpdater::ImGui() {

	ImGui::DragInt("startDivide", &start_.divide, 1, 3, 24);
	ImGui::DragInt("targetDivide", &target_.divide, 1, 3, 24);

	ImGui::DragInt("startUVMode", &start_.uvMode, 1, 0, 1);
	ImGui::DragInt("targetUVMode", &target_.uvMode, 1, 0, 1);

	ImGui::DragFloat("startOuterRadius", &start_.outerRadius, 0.01f);
	ImGui::DragFloat("targetOuterRadius", &target_.outerRadius, 0.01f);

	ImGui::DragFloat("startInnerRadius", &start_.innerRadius, 0.01f);
	ImGui::DragFloat("targetInnerRadius", &target_.innerRadius, 0.01f);

	ImGui::DragFloat("startStartAngle", &start_.startAngle, 0.01f);
	ImGui::DragFloat("targetStartAngle", &target_.startAngle, 0.01f);

	ImGui::DragFloat("startEndAngle", &start_.endAngle, 0.01f);
	ImGui::DragFloat("targetEndAngle", &target_.endAngle, 0.01f);

	ImGui::DragFloat("startLattice", &start_.lattice, 0.01f);
	ImGui::DragFloat("targetLattice", &target_.lattice, 0.01f);

	ImGui::DragFloat("startThickness", &start_.thickness, 0.01f, 0.1f, 8.0f);
	ImGui::DragFloat("targetThickness", &target_.thickness, 0.01f, 0.1f, 8.0f);

	ImGui::DragFloat2("startPivot", &start_.pivot.x, 0.01f);
	ImGui::DragFloat2("targetPivot", &target_.pivot.x, 0.01f);

	ImGui::ColorEdit4("startOuterColor", &start_.outerColor.r);
	ImGui::ColorEdit4("targetOuterColor", &target_.outerColor.r);

	ImGui::ColorEdit4("startInnerColor", &start_.innerColor.r);
	ImGui::ColorEdit4("targetInnerColor", &target_.innerColor.r);
}

void ParticleCrescentUpdater::FromJson(const Json& data) {

	const auto& crescentData = data["crescent"];

	start_.divide = crescentData.value("startDivide", start_.divide);
	target_.divide = crescentData.value("targetDivide", target_.divide);
	start_.uvMode = crescentData.value("startUVMode", start_.uvMode);
	target_.uvMode = crescentData.value("targetUVMode", target_.uvMode);

	start_.outerRadius = crescentData.value("startOuterRadius", start_.outerRadius);
	target_.outerRadius = crescentData.value("targetOuterRadius", target_.outerRadius);
	start_.innerRadius = crescentData.value("startInnerRadius", start_.innerRadius);
	target_.innerRadius = crescentData.value("targetInnerRadius", target_.innerRadius);

	start_.startAngle = crescentData.value("startStartAngle", start_.startAngle);
	target_.startAngle = crescentData.value("targetStartAngle", target_.startAngle);
	start_.endAngle = crescentData.value("startEndAngle", start_.endAngle);
	target_.endAngle = crescentData.value("targetEndAngle", target_.endAngle);

	start_.lattice = crescentData.value("startLattice", start_.lattice);
	target_.lattice = crescentData.value("targetLattice", target_.lattice);
	start_.thickness = crescentData.value("startThickness", start_.thickness);
	target_.thickness = crescentData.value("targetThickness", target_.thickness);

	start_.pivot = start_.pivot.FromJson(crescentData["startPivot"]);
	target_.pivot = target_.pivot.FromJson(crescentData["targetPivot"]);

	if (crescentData.contains("startOuterColor")) {

		start_.outerColor = start_.outerColor.FromJson(crescentData["startOuterColor"]);
		target_.outerColor = target_.outerColor.FromJson(crescentData["targetOuterColor"]);

		start_.innerColor = start_.innerColor.FromJson(crescentData["startInnerColor"]);
		target_.innerColor = target_.innerColor.FromJson(crescentData["targetInnerColor"]);
	}
}

void ParticleCrescentUpdater::ToJson(Json& data) const {

	data["crescent"]["startDivide"] = start_.divide;
	data["crescent"]["targetDivide"] = target_.divide;
	data["crescent"]["startUVMode"] = start_.uvMode;
	data["crescent"]["targetUVMode"] = target_.uvMode;

	data["crescent"]["startOuterRadius"] = start_.outerRadius;
	data["crescent"]["targetOuterRadius"] = target_.outerRadius;
	data["crescent"]["startInnerRadius"] = start_.innerRadius;
	data["crescent"]["targetInnerRadius"] = target_.innerRadius;

	data["crescent"]["startStartAngle"] = start_.startAngle;
	data["crescent"]["targetStartAngle"] = target_.startAngle;
	data["crescent"]["startEndAngle"] = start_.endAngle;
	data["crescent"]["targetEndAngle"] = target_.endAngle;

	data["crescent"]["startLattice"] = start_.lattice;
	data["crescent"]["targetLattice"] = target_.lattice;
	data["crescent"]["startThickness"] = start_.thickness;
	data["crescent"]["targetThickness"] = target_.thickness;

	data["crescent"]["startPivot"] = start_.pivot.ToJson();
	data["crescent"]["targetPivot"] = target_.pivot.ToJson();

	data["crescent"]["startOuterColor"] = start_.outerColor.ToJson();
	data["crescent"]["targetOuterColor"] = target_.outerColor.ToJson();

	data["crescent"]["startInnerColor"] = start_.innerColor.ToJson();
	data["crescent"]["targetInnerColor"] = target_.innerColor.ToJson();
}