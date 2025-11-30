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

	// 頂点カラーを更新
	particle.primitive.plane.leftTopVertexColor = Color::Lerp(
		start_.leftTopVertexColor,
		target_.leftTopVertexColor,
		EasedValue(easingType, particle.progress));
	particle.primitive.plane.rightTopVertexColor = Color::Lerp(
		start_.rightTopVertexColor,
		target_.rightTopVertexColor,
		EasedValue(easingType, particle.progress));
	particle.primitive.plane.leftBottomVertexColor = Color::Lerp(
		start_.leftBottomVertexColor,
		target_.leftBottomVertexColor,
		EasedValue(easingType, particle.progress));
	particle.primitive.plane.rightBottomVertexColor = Color::Lerp(
		start_.rightBottomVertexColor,
		target_.rightBottomVertexColor,
		EasedValue(easingType, particle.progress));
}

void ParticlePlaneUpdater::ImGui() {

	ImGui::DragFloat2("startSize", &start_.size.x, 0.01f);
	ImGui::DragFloat2("targetSize", &target_.size.x, 0.01f);

	ImGui::DragFloat2("startPivot", &start_.pivot.x, 0.01f);
	ImGui::DragFloat2("targetPivot", &target_.pivot.x, 0.01f);

	ImGui::ColorEdit4("startLeftTopVertexColor", &start_.leftTopVertexColor.r);
	ImGui::ColorEdit4("targetLeftTopVertexColor", &target_.leftTopVertexColor.r);

	ImGui::ColorEdit4("startRightTopVertexColor", &start_.rightTopVertexColor.r);
	ImGui::ColorEdit4("targetRightTopVertexColor", &target_.rightTopVertexColor.r);

	ImGui::ColorEdit4("startLeftBottomVertexColor", &start_.leftBottomVertexColor.r);
	ImGui::ColorEdit4("targetLeftBottomVertexColor", &target_.leftBottomVertexColor.r);

	ImGui::ColorEdit4("startRightBottomVertexColor", &start_.rightBottomVertexColor.r);
	ImGui::ColorEdit4("targetRightBottomVertexColor", &target_.rightBottomVertexColor.r);

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

	start_.leftTopVertexColor = Color::FromJson(planeData.value("startLeftTopVertexColor", Json()));
	target_.leftTopVertexColor = Color::FromJson(planeData.value("targetLeftTopVertexColor", Json()));
	start_.rightTopVertexColor = Color::FromJson(planeData.value("startRightTopVertexColor", Json()));
	target_.rightTopVertexColor = Color::FromJson(planeData.value("targetRightTopVertexColor", Json()));
	start_.leftBottomVertexColor = Color::FromJson(planeData.value("startLeftBottomVertexColor", Json()));
	target_.leftBottomVertexColor = Color::FromJson(planeData.value("targetLeftBottomVertexColor", Json()));
	start_.rightBottomVertexColor = Color::FromJson(planeData.value("startRightBottomVertexColor", Json()));
	target_.rightBottomVertexColor = Color::FromJson(planeData.value("targetRightBottomVertexColor", Json()));
}

void ParticlePlaneUpdater::ToJson(Json& data) const {

	data["plane"]["startSize"] = start_.size.ToJson();
	data["plane"]["targetSize"] = target_.size.ToJson();
	data["plane"]["startPivot"] = start_.pivot.ToJson();
	data["plane"]["targetPivot"] = target_.pivot.ToJson();
	data["plane"]["mode"] = EnumAdapter<ParticlePlaneType>::ToString(planeType_);

	data["plane"]["startLeftTopVertexColor"] = start_.leftTopVertexColor.ToJson();
	data["plane"]["targetLeftTopVertexColor"] = target_.leftTopVertexColor.ToJson();
	data["plane"]["startRightTopVertexColor"] = start_.rightTopVertexColor.ToJson();
	data["plane"]["targetRightTopVertexColor"] = target_.rightTopVertexColor.ToJson();
	data["plane"]["startLeftBottomVertexColor"] = start_.leftBottomVertexColor.ToJson();
	data["plane"]["targetLeftBottomVertexColor"] = target_.leftBottomVertexColor.ToJson();
	data["plane"]["startRightBottomVertexColor"] = start_.rightBottomVertexColor.ToJson();
	data["plane"]["targetRightBottomVertexColor"] = target_.rightBottomVertexColor.ToJson();
}