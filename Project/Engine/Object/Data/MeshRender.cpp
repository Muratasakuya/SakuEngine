#include "MeshRender.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	MeshRender classMethods
//============================================================================

void MeshRender::Init(const std::string& name) {

	// 名前を設定
	modelName = name;

	// デフォルトの描画先
	renderView = MeshRenderView::Both;
}

void MeshRender::ImGui(float itemSize) {

	ImGui::PushItemWidth(itemSize);

	ImGui::Text("modelName: %s", modelName.c_str());
	EnumAdapter<MeshRenderView>::Combo("RenderView", &renderView);

	ImGui::PopItemWidth();
}

void MeshRender::FromJson(const Json& data) {

	renderView = EnumAdapter<MeshRenderView>::FromString(
		data.value("renderView", "Both")).value();
}

void MeshRender::ToJson(Json& data) {

	data["renderView"] = EnumAdapter<MeshRenderView>::ToString(renderView);
}