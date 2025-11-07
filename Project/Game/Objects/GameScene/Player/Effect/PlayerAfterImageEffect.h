#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>
#include <Engine/MathLib/Vector4.h>

//============================================================================
//	PlayerAfterImageEffect class
//	オブジェクトの残像表現エフェクトを呼びだして処理させるクラス
//============================================================================
class PlayerAfterImageEffect {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerAfterImageEffect() = default;
	~PlayerAfterImageEffect() = default;

	// 初期化
	void Init(const std::string& fileName);

	// エディター
	void ImGui(bool isSeparate = true);

	// 色、アウトラインの太さを指定してエフェクトを開始
	void Start(std::vector<GameObject3D*>& objects);
	// 終了
	void End(std::vector<GameObject3D*>& objects);

	//--------- accessor -----------------------------------------------------

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// ファイルの名前
	std::string fileName_;

	// 色
	Color color_;
	// アウトラインの太さ
	float edgeScale_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};