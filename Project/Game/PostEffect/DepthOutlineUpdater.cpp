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

	// Resetで効果を消す
	Reset();
}

void DepthOutlineUpdater::Update() {

	// 常に更新する

	// シーンから受け取った値で更新する
	bufferData_.projectionInverse = Matrix4x4::Inverse(sceneView_->GetCamera()->GetProjectionMatrix());
}

void DepthOutlineUpdater::Start(const Color& color, float edgeScale) {

	// 太さ、色を指定
	bufferData_.edgeScale = edgeScale;
	bufferData_.color = Vector3(color.r, color.g, color.b);
}

void DepthOutlineUpdater::Reset() {

	// 太さを0.0fにして効果を消す
	bufferData_.edgeScale = 0.0f;
}

void DepthOutlineUpdater::ImGui() {

	SaveButton();

	ImGui::DragFloat("edgeScale", &bufferData_.edgeScale, 0.01f);
	ImGui::DragFloat("threshold", &bufferData_.threshold, 0.01f);
	ImGui::ColorEdit3("color", &bufferData_.color.x);
}

void DepthOutlineUpdater::ApplyJson() {

	Json data;
	if (!LoadFile(data)) {
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

	SaveFile(data);
}