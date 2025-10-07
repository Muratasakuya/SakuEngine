#include "DepthOutlineUpdater.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/SceneView.h>

//============================================================================
//	DepthOutlineUpdater classMethods
//============================================================================

void DepthOutlineUpdater::Init() {

	bufferData_ = DepthBasedOutlineForGPU{};

	// json適応
	ApplyJson();
}

void DepthOutlineUpdater::Update() {

	// 常に更新する

	// シーンから受け取った値で更新する
	bufferData_.projectionInverse = Matrix4x4::Inverse(sceneView_->GetCamera()->GetProjectionMatrix());
}

void DepthOutlineUpdater::ImGui() {

	SaveButton();

	ImGui::DragFloat("edgeScale", &bufferData_.edgeScale, 0.01f);
	ImGui::DragFloat("threshold", &bufferData_.threshold, 0.01f);
	ImGui::ColorEdit3("color", &bufferData_.color.x);
}

void DepthOutlineUpdater::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck(kJsonBasePath_ + "depthOutlineUpdater.json", data)) {
		return;
	}

	bufferData_.edgeScale = data.value("edgeScale", 1.0f);
	bufferData_.threshold = data.value("threshold", 1.0f);
	bufferData_.color = Vector3::FromJson(data.value("color", Json()));
}

void DepthOutlineUpdater::SaveJson() {

	Json data;

	data["edgeScale"] = bufferData_.edgeScale;
	data["threshold"] = bufferData_.threshold;
	data["color"] = bufferData_.color.ToJson();

	JsonAdapter::Save(kJsonBasePath_ + "depthOutlineUpdater.json", data);
}