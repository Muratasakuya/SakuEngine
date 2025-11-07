#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/DxStructures.h>
#include <Engine/Object/Data/MeshRenderStructure.h>
#include <Engine/MathLib/MathUtils.h>

//============================================================================
//	MeshRender class
//	描画設定をRendererに伝えるためのデータ
//============================================================================
class MeshRender {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	MeshRender() = default;
	~MeshRender() = default;

	// 初期化
	void Init(const std::string& name);

	// エディター
	void ImGui(float itemSize);

	// json
	void ToJson(Json& data);
	void FromJson(const Json& data);

	//--------- variables ----------------------------------------------------

	// 描画しているモデルの名前
	std::string modelName;

	// 描画先
	MeshRenderView renderView;

	// ブレンドモード
	BlendMode blendMode;
};