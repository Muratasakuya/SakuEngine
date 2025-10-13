#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector3.h>

// c++
#include <vector>

//============================================================================
//	SerialUVScroll class
//============================================================================
class SerialUVScroll {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SerialUVScroll() = default;
	~SerialUVScroll() = default;

	void Update(float progress, Vector3& translation, Vector3& scale);

	void ImGui();

	// json
	void FromJson(const Json& data);
	void ToJson(Json& data) const;

	//--------- accessor -----------------------------------------------------

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// スクロール開始位置
	enum class StartCorner {

		LeftTop,     // 左上
		RightTop,    // 右上
		LeftBottom,  // 左下
		RightBottom, // 右下
	};
	// 横の初期進行方向
	enum class HorizontalDirection {

		Right,
		Left
	};
	// 縦に折り返す方向
	enum class VerticalDirection {

		Down,
		Up
	};

	// 巡回モード
	enum class Traverse {

		Wrap,  // 毎行同じ水平方向で端まで行ったら縦に1つ進む
		Snake, // 行ごとに左右の進行方向を反転する
	};

	struct Vector2Int {

		int x;
		int y;
	};

	// 巡回マス
	struct Cell {

		int col;
		int row;
	};

	//--------- variables ----------------------------------------------------

	 // ループ回数
	int loopCount_ = 1;

	// グリッド
	int cols_ = 1; // 縦
	int rows_ = 1; // 横
	int rowsUse_ = 0; // 使う行数

	// 横に進む数
	std::vector<int> horizontalCounts_;
	// 巡回マス
	std::vector<Cell> path_;
	bool dirty_ = true;

	// レイアウト
	Vector2Int textureSize_; // 実際のテクスチャサイズ
	Vector2Int cellSize_;    // 1コマのサイズ
	Vector2Int origin_;      // 最初のコマ座標
	Vector2Int gapSize_;     // コマ間の隙間

	// 開始位置・進行方法
	StartCorner startCorner_ = StartCorner::LeftTop;
	HorizontalDirection horizontal_ = HorizontalDirection::Right;
	VerticalDirection vertical_ = VerticalDirection::Down;
	Traverse traverse_ = Traverse::Wrap;

	//--------- functions ----------------------------------------------------

	// helper
	void BuildPath();
	uint32_t GetTotalFrames() const { return static_cast<uint32_t>(path_.size()); }
};