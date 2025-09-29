#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/MeshRenderStructure.h>
#include <Engine/MathLib/MathUtils.h>

//============================================================================
//	MeshRender class
//============================================================================
class MeshRender {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	MeshRender() = default;
	~MeshRender() = default;

	void Init(const std::string& name);

	void ImGui(float itemSize);

	void ToJson(Json& data);
	void FromJson(const Json& data);

	//--------- variables ----------------------------------------------------

	// 描画しているモデルの名前
	std::string modelName;

	// 描画先
	MeshRenderView renderView;
};