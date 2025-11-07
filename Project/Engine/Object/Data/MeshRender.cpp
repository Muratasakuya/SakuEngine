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

	// デフォルト
	renderView = MeshRenderView::Both;
	blendMode = BlendMode::kBlendModeNormal;
}

void MeshRender::ImGui(float itemSize) {

	ImGui::PushItemWidth(itemSize);

	ImGui::Text("modelName: %s", modelName.c_str());
	EnumAdapter<MeshRenderView>::Combo("RenderView", &renderView);
	EnumAdapter<BlendMode>::Combo("BlendMode", &blendMode);

	ImGui::PopItemWidth();
}

void MeshRender::FromJson(const Json& data) {

	renderView = EnumAdapter<MeshRenderView>::FromString(
		data.value("renderView", "Both")).value();
	blendMode = EnumAdapter<BlendMode>::FromString(
		data.value("blendMode", "kBlendModeNormal")).value();
}

void MeshRender::ToJson(Json& data) {

	data["renderView"] = EnumAdapter<MeshRenderView>::ToString(renderView);
	data["blendMode"] = EnumAdapter<BlendMode>::ToString(blendMode);
}