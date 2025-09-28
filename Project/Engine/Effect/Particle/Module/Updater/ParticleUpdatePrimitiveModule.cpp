#include "ParticleUpdatePrimitiveModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/Algorithm.h>

//============================================================================
//	ParticleUpdatePrimitiveModule classMethods
//============================================================================

void ParticleUpdatePrimitiveModule::Init() {

	// 初期化値、全ての形状を初期化
	primitive_.start.plane.Init();
	primitive_.target.plane.Init();

	primitive_.start.ring.Init();
	primitive_.target.ring.Init();

	primitive_.start.cylinder.Init();
	primitive_.target.cylinder.Init();

	primitive_.start.crescent.Init();
	primitive_.target.crescent.Init();

	planeType_ = ParticlePlaneType::XY;
}

void ParticleUpdatePrimitiveModule::Execute(
	CPUParticle::ParticleData& particle, [[maybe_unused]] float deltaTime) {

	// 形状別で補間
	type_ = particle.primitive.type;
	switch (type_) {
	case ParticlePrimitiveType::Plane: {

		UpdatePlane(particle);
		break;
	}
	case ParticlePrimitiveType::Ring: {

		UpdateRing(particle);
		break;
	}
	case ParticlePrimitiveType::Cylinder: {

		UpdateCylinder(particle);
		break;
	}
	case ParticlePrimitiveType::Crescent: {

		UpdateCrescent(particle);
		break;
	}
	}
}

//============================================================================
//	Plane
//============================================================================
void ParticleUpdatePrimitiveModule::UpdatePlane(CPUParticle::ParticleData& particle) {

	particle.primitive.plane.mode = primitive_.start.plane.mode;
	particle.primitive.plane.mode = primitive_.target.plane.mode;

	particle.primitive.plane.size = Vector2::Lerp(primitive_.start.plane.size,
		primitive_.target.plane.size, EasedValue(easingType_, particle.progress));

	particle.primitive.plane.pivot = Vector2::Lerp(primitive_.start.plane.pivot,
		primitive_.target.plane.pivot, EasedValue(easingType_, particle.progress));
}

//============================================================================
//	Ring
//============================================================================
void ParticleUpdatePrimitiveModule::UpdateRing(CPUParticle::ParticleData& particle) {

	particle.primitive.ring.divide = Algorithm::LerpInt(primitive_.start.ring.divide,
		primitive_.target.ring.divide, EasedValue(easingType_, particle.progress));

	particle.primitive.ring.outerRadius = std::lerp(primitive_.start.ring.outerRadius,
		primitive_.target.ring.outerRadius, EasedValue(easingType_, particle.progress));

	particle.primitive.ring.innerRadius = std::lerp(primitive_.start.ring.innerRadius,
		primitive_.target.ring.innerRadius, EasedValue(easingType_, particle.progress));
}

//============================================================================
//	Cylinder
//============================================================================
void ParticleUpdatePrimitiveModule::UpdateCylinder(CPUParticle::ParticleData& particle) {

	particle.primitive.cylinder.divide = Algorithm::LerpInt(primitive_.start.cylinder.divide,
		primitive_.target.cylinder.divide, EasedValue(easingType_, particle.progress));

	particle.primitive.cylinder.topRadius = std::lerp(primitive_.start.cylinder.topRadius,
		primitive_.target.cylinder.topRadius, EasedValue(easingType_, particle.progress));

	particle.primitive.cylinder.bottomRadius = std::lerp(primitive_.start.cylinder.bottomRadius,
		primitive_.target.cylinder.bottomRadius, EasedValue(easingType_, particle.progress));

	particle.primitive.cylinder.height = std::lerp(primitive_.start.cylinder.height,
		primitive_.target.cylinder.height, EasedValue(easingType_, particle.progress));
}

//============================================================================
//	Crescent
//============================================================================
void ParticleUpdatePrimitiveModule::UpdateCrescent(CPUParticle::ParticleData& particle) {

	particle.primitive.crescent.uvMode = primitive_.start.crescent.uvMode;
	particle.primitive.crescent.uvMode = primitive_.target.crescent.uvMode;

	particle.primitive.crescent.divide = Algorithm::LerpInt(primitive_.start.crescent.divide,
		primitive_.target.crescent.divide, EasedValue(easingType_, particle.progress));

	particle.primitive.crescent.outerRadius = std::lerp(primitive_.start.crescent.outerRadius,
		primitive_.target.crescent.outerRadius, EasedValue(easingType_, particle.progress));

	particle.primitive.crescent.innerRadius = std::lerp(primitive_.start.crescent.innerRadius,
		primitive_.target.crescent.innerRadius, EasedValue(easingType_, particle.progress));

	particle.primitive.crescent.startAngle = std::lerp(primitive_.start.crescent.startAngle,
		primitive_.target.crescent.startAngle, EasedValue(easingType_, particle.progress));

	particle.primitive.crescent.endAngle = std::lerp(primitive_.start.crescent.endAngle,
		primitive_.target.crescent.endAngle, EasedValue(easingType_, particle.progress));

	particle.primitive.crescent.lattice = std::lerp(primitive_.start.crescent.lattice,
		primitive_.target.crescent.lattice, EasedValue(easingType_, particle.progress));

	particle.primitive.crescent.thickness = std::lerp(primitive_.start.crescent.thickness,
		primitive_.target.crescent.thickness, EasedValue(easingType_, particle.progress));

	particle.primitive.crescent.pivot = Vector2::Lerp(primitive_.start.crescent.pivot,
		primitive_.target.crescent.pivot, EasedValue(easingType_, particle.progress));

	particle.primitive.crescent.outerColor = Color::Lerp(primitive_.start.crescent.outerColor,
		primitive_.target.crescent.outerColor, EasedValue(easingType_, particle.progress));

	particle.primitive.crescent.innerColor = Color::Lerp(primitive_.start.crescent.innerColor,
		primitive_.target.crescent.innerColor, EasedValue(easingType_, particle.progress));
}

void ParticleUpdatePrimitiveModule::ImGui() {

	Easing::SelectEasingType(easingType_, GetName());

	switch (type_) {
	case ParticlePrimitiveType::Plane: {

		ImGui::DragFloat2("startSize", &primitive_.start.plane.size.x, 0.01f);
		ImGui::DragFloat2("targetSize", &primitive_.target.plane.size.x, 0.01f);

		ImGui::DragFloat2("startPivot", &primitive_.start.plane.pivot.x, 0.01f);
		ImGui::DragFloat2("targetPivot", &primitive_.target.plane.pivot.x, 0.01f);

		if (EnumAdapter<ParticlePlaneType>::Combo("planeType", &planeType_)) {

			primitive_.start.plane.mode = static_cast<uint32_t>(planeType_);
			primitive_.target.plane.mode = static_cast<uint32_t>(planeType_);
		}
		break;
	}
	case ParticlePrimitiveType::Ring: {

		ImGui::DragInt("startDivide", &primitive_.start.ring.divide, 1, 3, 32);
		ImGui::DragInt("targetDivide", &primitive_.target.ring.divide, 1, 3, 32);

		ImGui::DragFloat("startOuterRadius", &primitive_.start.ring.outerRadius, 0.01f);
		ImGui::DragFloat("targetOuterRadius", &primitive_.target.ring.outerRadius, 0.01f);

		ImGui::DragFloat("startInnerRadius", &primitive_.start.ring.innerRadius, 0.01f);
		ImGui::DragFloat("targetInnerRadius", &primitive_.target.ring.innerRadius, 0.01f);
		break;
	}
	case ParticlePrimitiveType::Cylinder: {

		ImGui::DragInt("startDivide", &primitive_.start.cylinder.divide, 1, 3, 32);
		ImGui::DragInt("targetDivide", &primitive_.target.cylinder.divide, 1, 3, 32);

		ImGui::DragFloat("startTopRadius", &primitive_.start.cylinder.topRadius, 0.01f);
		ImGui::DragFloat("targetTopRadius", &primitive_.target.cylinder.topRadius, 0.01f);

		ImGui::DragFloat("startTopBottomRadius", &primitive_.start.cylinder.bottomRadius, 0.01f);
		ImGui::DragFloat("targetBottomRadius", &primitive_.target.cylinder.bottomRadius, 0.01f);

		ImGui::DragFloat("startHeight", &primitive_.start.cylinder.height, 0.01f);
		ImGui::DragFloat("targetHeight", &primitive_.target.cylinder.height, 0.01f);
		break;
	}
	case ParticlePrimitiveType::Crescent: {

		ImGui::DragInt("startDivide", &primitive_.start.crescent.divide, 1, 3, 24);
		ImGui::DragInt("targetDivide", &primitive_.target.crescent.divide, 1, 3, 24);

		ImGui::DragInt("startUVMode", &primitive_.start.crescent.uvMode, 1, 0, 1);
		ImGui::DragInt("targetUVMode", &primitive_.target.crescent.uvMode, 1, 0, 1);

		ImGui::DragFloat("startOuterRadius", &primitive_.start.crescent.outerRadius, 0.01f);
		ImGui::DragFloat("targetOuterRadius", &primitive_.target.crescent.outerRadius, 0.01f);

		ImGui::DragFloat("startInnerRadius", &primitive_.start.crescent.innerRadius, 0.01f);
		ImGui::DragFloat("targetInnerRadius", &primitive_.target.crescent.innerRadius, 0.01f);

		ImGui::DragFloat("startStartAngle", &primitive_.start.crescent.startAngle, 0.01f);
		ImGui::DragFloat("targetStartAngle", &primitive_.target.crescent.startAngle, 0.01f);

		ImGui::DragFloat("startEndAngle", &primitive_.start.crescent.endAngle, 0.01f);
		ImGui::DragFloat("targetEndAngle", &primitive_.target.crescent.endAngle, 0.01f);

		ImGui::DragFloat("startLattice", &primitive_.start.crescent.lattice, 0.01f);
		ImGui::DragFloat("targetLattice", &primitive_.target.crescent.lattice, 0.01f);

		ImGui::DragFloat("startThickness", &primitive_.start.crescent.thickness, 0.01f, 0.1f, 8.0f);
		ImGui::DragFloat("targetThickness", &primitive_.target.crescent.thickness, 0.01f, 0.1f, 8.0f);

		ImGui::DragFloat2("startPivot", &primitive_.start.crescent.pivot.x, 0.01f);
		ImGui::DragFloat2("targetPivot", &primitive_.target.crescent.pivot.x, 0.01f);

		ImGui::ColorEdit4("startOuterColor", &primitive_.start.crescent.outerColor.r);
		ImGui::ColorEdit4("targetOuterColor", &primitive_.target.crescent.outerColor.r);

		ImGui::ColorEdit4("startInnerColor", &primitive_.start.crescent.innerColor.r);
		ImGui::ColorEdit4("targetInnerColor", &primitive_.target.crescent.innerColor.r);
		break;
	}
	}
}

Json ParticleUpdatePrimitiveModule::ToJson() {

	Json data;

	// イージング
	data["easingType"] = EnumAdapter<EasingType>::ToString(easingType_);

	// Plane
	{
		data["plane"]["startSize"] = primitive_.start.plane.size.ToJson();
		data["plane"]["targetSize"] = primitive_.target.plane.size.ToJson();
		data["plane"]["startPivot"] = primitive_.start.plane.pivot.ToJson();
		data["plane"]["targetPivot"] = primitive_.target.plane.pivot.ToJson();
		data["plane"]["mode"] = EnumAdapter<ParticlePlaneType>::ToString(planeType_);
	}

	// Ring
	{
		data["ring"]["startDivide"] = primitive_.start.ring.divide;
		data["ring"]["targetDivide"] = primitive_.target.ring.divide;
		data["ring"]["startOuterRadius"] = primitive_.start.ring.outerRadius;
		data["ring"]["targetOuterRadius"] = primitive_.target.ring.outerRadius;
		data["ring"]["startInnerRadius"] = primitive_.start.ring.innerRadius;
		data["ring"]["targetInnerRadius"] = primitive_.target.ring.innerRadius;
	}

	// Cylinder
	{
		data["cylinder"]["startDivide"] = primitive_.start.cylinder.divide;
		data["cylinder"]["targetDivide"] = primitive_.target.cylinder.divide;
		data["cylinder"]["startTopRadius"] = primitive_.start.cylinder.topRadius;
		data["cylinder"]["targetTopRadius"] = primitive_.target.cylinder.topRadius;
		data["cylinder"]["startBottomRadius"] = primitive_.start.cylinder.bottomRadius;
		data["cylinder"]["targetBottomRadius"] = primitive_.target.cylinder.bottomRadius;
		data["cylinder"]["startHeight"] = primitive_.start.cylinder.height;
		data["cylinder"]["targetHeight"] = primitive_.target.cylinder.height;
	}

	// Crescent
	{
		data["crescent"]["startDivide"] = primitive_.start.crescent.divide;
		data["crescent"]["targetDivide"] = primitive_.target.crescent.divide;
		data["crescent"]["startUVMode"] = primitive_.start.crescent.uvMode;
		data["crescent"]["targetUVMode"] = primitive_.target.crescent.uvMode;

		data["crescent"]["startOuterRadius"] = primitive_.start.crescent.outerRadius;
		data["crescent"]["targetOuterRadius"] = primitive_.target.crescent.outerRadius;
		data["crescent"]["startInnerRadius"] = primitive_.start.crescent.innerRadius;
		data["crescent"]["targetInnerRadius"] = primitive_.target.crescent.innerRadius;

		data["crescent"]["startStartAngle"] = primitive_.start.crescent.startAngle;
		data["crescent"]["targetStartAngle"] = primitive_.target.crescent.startAngle;
		data["crescent"]["startEndAngle"] = primitive_.start.crescent.endAngle;
		data["crescent"]["targetEndAngle"] = primitive_.target.crescent.endAngle;

		data["crescent"]["startLattice"] = primitive_.start.crescent.lattice;
		data["crescent"]["targetLattice"] = primitive_.target.crescent.lattice;
		data["crescent"]["startThickness"] = primitive_.start.crescent.thickness;
		data["crescent"]["targetThickness"] = primitive_.target.crescent.thickness;

		data["crescent"]["startPivot"] = primitive_.start.crescent.pivot.ToJson();
		data["crescent"]["targetPivot"] = primitive_.target.crescent.pivot.ToJson();

		data["crescent"]["startOuterColor"] = primitive_.start.crescent.outerColor.ToJson();
		data["crescent"]["targetOuterColor"] = primitive_.target.crescent.outerColor.ToJson();

		data["crescent"]["startInnerColor"] = primitive_.start.crescent.innerColor.ToJson();
		data["crescent"]["targetInnerColor"] = primitive_.target.crescent.innerColor.ToJson();
	}

	return data;
}

void ParticleUpdatePrimitiveModule::FromJson(const Json& data) {

	// イージング
	const auto& easingType = EnumAdapter<EasingType>::FromString(data.value("easingType", ""));
	easingType_ = easingType.value();

	// Plane
	{
		const auto& planeData = data["plane"];
		primitive_.start.plane.size = primitive_.start.plane.size.FromJson(planeData["startSize"]);
		primitive_.target.plane.size = primitive_.target.plane.size.FromJson(planeData["targetSize"]);
		primitive_.start.plane.pivot = primitive_.start.plane.pivot.FromJson(planeData["startPivot"]);
		primitive_.target.plane.pivot = primitive_.target.plane.pivot.FromJson(planeData["targetPivot"]);

		const auto& planeType = EnumAdapter<ParticlePlaneType>::FromString(data["plane"]["mode"]);
		primitive_.start.plane.mode = static_cast<uint32_t>(planeType.value());
		primitive_.target.plane.mode = static_cast<uint32_t>(planeType.value());
	}

	// Ring
	{
		const auto& ringData = data["ring"];
		primitive_.start.ring.divide = ringData.value("startDivide", primitive_.start.ring.divide);
		primitive_.target.ring.divide = ringData.value("targetDivide", primitive_.target.ring.divide);
		primitive_.start.ring.outerRadius = ringData.value("startOuterRadius", primitive_.start.ring.outerRadius);
		primitive_.target.ring.outerRadius = ringData.value("targetOuterRadius", primitive_.target.ring.outerRadius);
		primitive_.start.ring.innerRadius = ringData.value("startInnerRadius", primitive_.start.ring.innerRadius);
		primitive_.target.ring.innerRadius = ringData.value("targetInnerRadius", primitive_.target.ring.innerRadius);
	}

	// Cylinder
	{
		const auto& cylinderData = data["cylinder"];
		primitive_.start.cylinder.divide = cylinderData.value("startDivide", primitive_.start.cylinder.divide);
		primitive_.target.cylinder.divide = cylinderData.value("targetDivide", primitive_.target.cylinder.divide);
		primitive_.start.cylinder.topRadius = cylinderData.value("startTopRadius", primitive_.start.cylinder.topRadius);
		primitive_.target.cylinder.topRadius = cylinderData.value("targetTopRadius", primitive_.target.cylinder.topRadius);
		primitive_.start.cylinder.bottomRadius = cylinderData.value("startBottomRadius", primitive_.start.cylinder.bottomRadius);
		primitive_.target.cylinder.bottomRadius = cylinderData.value("targetBottomRadius", primitive_.target.cylinder.bottomRadius);
		primitive_.start.cylinder.height = cylinderData.value("startHeight", primitive_.start.cylinder.height);
		primitive_.target.cylinder.height = cylinderData.value("targetHeight", primitive_.target.cylinder.height);
	}

	// Crescent
	{
		const auto& crescentData = data["crescent"];

		primitive_.start.crescent.divide = crescentData.value("startDivide", primitive_.start.crescent.divide);
		primitive_.target.crescent.divide = crescentData.value("targetDivide", primitive_.target.crescent.divide);
		primitive_.start.crescent.uvMode = crescentData.value("startUVMode", primitive_.start.crescent.uvMode);
		primitive_.target.crescent.uvMode = crescentData.value("targetUVMode", primitive_.target.crescent.uvMode);

		primitive_.start.crescent.outerRadius = crescentData.value("startOuterRadius", primitive_.start.crescent.outerRadius);
		primitive_.target.crescent.outerRadius = crescentData.value("targetOuterRadius", primitive_.target.crescent.outerRadius);
		primitive_.start.crescent.innerRadius = crescentData.value("startInnerRadius", primitive_.start.crescent.innerRadius);
		primitive_.target.crescent.innerRadius = crescentData.value("targetInnerRadius", primitive_.target.crescent.innerRadius);

		primitive_.start.crescent.startAngle = crescentData.value("startStartAngle", primitive_.start.crescent.startAngle);
		primitive_.target.crescent.startAngle = crescentData.value("targetStartAngle", primitive_.target.crescent.startAngle);
		primitive_.start.crescent.endAngle = crescentData.value("startEndAngle", primitive_.start.crescent.endAngle);
		primitive_.target.crescent.endAngle = crescentData.value("targetEndAngle", primitive_.target.crescent.endAngle);

		primitive_.start.crescent.lattice = crescentData.value("startLattice", primitive_.start.crescent.lattice);
		primitive_.target.crescent.lattice = crescentData.value("targetLattice", primitive_.target.crescent.lattice);
		primitive_.start.crescent.thickness = crescentData.value("startThickness", primitive_.start.crescent.thickness);
		primitive_.target.crescent.thickness = crescentData.value("targetThickness", primitive_.target.crescent.thickness);

		primitive_.start.crescent.pivot = primitive_.start.crescent.pivot.FromJson(crescentData["startPivot"]);
		primitive_.target.crescent.pivot = primitive_.target.crescent.pivot.FromJson(crescentData["targetPivot"]);

		if (crescentData.contains("startOuterColor")) {

			primitive_.start.crescent.outerColor = primitive_.start.crescent.outerColor.FromJson(crescentData["startOuterColor"]);
			primitive_.target.crescent.outerColor = primitive_.target.crescent.outerColor.FromJson(crescentData["targetOuterColor"]);

			primitive_.start.crescent.innerColor = primitive_.start.crescent.innerColor.FromJson(crescentData["startInnerColor"]);
			primitive_.target.crescent.innerColor = primitive_.target.crescent.innerColor.FromJson(crescentData["targetInnerColor"]);
		}
	}
}