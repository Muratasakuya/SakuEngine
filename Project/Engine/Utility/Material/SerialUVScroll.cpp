#include "SerialUVScroll.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	SerialUVScroll classMethods
//============================================================================

static inline float UVx(int px, int textureSizeX) {

	return textureSizeX ? static_cast<float>(px) / static_cast<float>(textureSizeX) : 0.0f;
}
static inline float UVy(int px, int textureSizeY) {

	return textureSizeY ? static_cast<float>(px) / static_cast<float>(textureSizeY) : 0.0f;
}

void SerialUVScroll::BuildPath() {

	path_.clear();
	if (cols_ <= 0 || rows_ <= 0) {
		return;
	}

	// 横の数
	const int useRows = (rowsUse_ > 0 && rowsUse_ <= rows_) ? rowsUse_ : rows_;

	// 行ごとの横コマ数
	std::vector<int> perRow = horizontalCounts_;
	if (static_cast<int>(perRow.size()) < useRows) {

		perRow.resize(useRows, 0);
	}
	for (int row = 0; row < useRows; ++row) {
		if (perRow[row] <= 0) {

			perRow[row] = cols_;
		}
	}

	// 横
	auto rowIndex = [&](int i)->int {

		// 開始地点が下かどうか
		bool invertY = (startCorner_ == StartCorner::LeftBottom) ||
			(startCorner_ == StartCorner::RightBottom);
		//  開始地点で上下反転する
		int base = (vertical_ == VerticalDirection::Down) ? i : (useRows - 1 - i);
		return invertY ? (rows_ - 1 - base) : base; };
	// 縦
	auto colIndex = [&](int i, bool invert)->int {

		// 開始地点が右かどうか
		bool invertX = (startCorner_ == StartCorner::RightTop) ||
			(startCorner_ == StartCorner::RightBottom);
		// 開始地点で左右反転する
		int base = invert ? (cols_ - 1 - i) : i;
		return invertX ? (cols_ - 1 - base) : base; };

	// 左開始かどうか
	bool hInvert = (horizontal_ == HorizontalDirection::Left);
	for (int rIndex = 0; rIndex < useRows; ++rIndex) {

		const int r = rowIndex(rIndex);
		const int count = (std::min)(perRow[rIndex], cols_);
		for (int cIndex = 0; cIndex < count; ++cIndex) {

			const int c = colIndex(cIndex, hInvert);
			// マスを設定
			path_.push_back({ c, r });
		}
		// 次の行に進む
		if (traverse_ == Traverse::Snake) {

			hInvert = !hInvert;
		}
	}
	// 作成済み
	dirty_ = false;
}

void SerialUVScroll::Update(float progress, Vector3& translation, Vector3& scale) {

	// 未作成なら巡回パスを設定
	if (dirty_) {

		BuildPath();
	}
	const int n = GetTotalFrames();
	// 0以下ならなにも処理しないでそのまま返す
	if (n <= 0) {
		return;
	}

	// ループ回数を掛け合わせてに正規化
	const float looped = std::fmod((std::max)(0.0f, progress) * (std::max)(1, loopCount_), 1.0f);
	const int index = (std::min)(n - 1, static_cast<int>(std::floor(looped * n)));
	const Cell cell = path_[index];

	// 1コマのUVスケール値
	if (textureSize_.x > 0 && textureSize_.y > 0 &&
		cellSize_.x > 0 && cellSize_.y > 0) {

		scale.x = static_cast<float>(cellSize_.x) / static_cast<float>(textureSize_.x);
		scale.y = static_cast<float>(cellSize_.y) / static_cast<float>(textureSize_.y);
	} else {

		// サイズ未指定なら均等に割る
		scale.x = (0 < cols_) ? 1.0f / static_cast<float>(cols_) : 1.0f;
		scale.y = (0 < rows_) ? 1.0f / static_cast<float>(rows_) : 1.0f;
	}
	scale.z = 1.0f;

	// このコマの左上UV座標
	if (textureSize_.x > 0 && textureSize_.y > 0 &&
		cellSize_.x > 0 && cellSize_.y > 0) {

		const int px = origin_.x + cell.col * (cellSize_.x + gapSize_.x);
		const int py = origin_.y + cell.row * (cellSize_.y + gapSize_.y);
		translation.x = UVx(px, textureSize_.x);
		translation.y = UVy(py, textureSize_.y);
	} else {

		translation.x = static_cast<float>(cell.col) * scale.x;
		translation.y = static_cast<float>(cell.row) * scale.y;
	}
	translation.z = 0.0f;
}

void SerialUVScroll::ImGui() {

	{
		ImGui::SeparatorText("Sprite Sheet");

		ImGui::DragInt("loopCount", &loopCount_, 1, 1);
		ImGui::DragInt("cols", &cols_, 1, 1);
		ImGui::DragInt("rows", &rows_, 1, 1);
		ImGui::DragInt("rowsUse", &rowsUse_, 1, 0, rows_);
	}
	{
		ImGui::SeparatorText("Layout");
		ImGui::DragInt2("textureSize", &textureSize_.x, 1, 0);
		ImGui::DragInt2("cellSize", &cellSize_.x, 1, 0);
		ImGui::DragInt2("origin", &origin_.x, 1, 0);
		ImGui::DragInt2("gapSize", &gapSize_.x, 1, 0);
	}
	{
		ImGui::SeparatorText("Traverse");

		if (EnumAdapter<StartCorner>::Combo("StartCorner", &startCorner_)) {

			dirty_ = true;
		}
		if (EnumAdapter<HorizontalDirection>::Combo("Horizontal", &horizontal_)) {

			dirty_ = true;
		}
		if (EnumAdapter<VerticalDirection>::Combo("Vertical", &vertical_)) {

			dirty_ = true;
		}
		if (EnumAdapter<Traverse>::Combo("Traverse", &traverse_)) {

			dirty_ = true;
		}
	}
	{
		// 行ごとの横コマ数
		if (rowsUse_ <= 0) {

			rowsUse_ = rows_;
		}
		// 数が異なるときはリサイズして調整する
		if (static_cast<int>(horizontalCounts_.size()) != rowsUse_) {

			horizontalCounts_.resize(rowsUse_, cols_);
		}
		if (ImGui::TreeNode("horizontalCounts")) {
			for (int i = 0; i < rowsUse_; ++i) {

				ImGui::DragInt(("row " + std::to_string(i)).c_str(), &horizontalCounts_[i], 1, 0, cols_);
			}
			ImGui::TreePop();
		}
	}
	if (ImGui::Button("Rebuild Path")) {

		dirty_ = true;
	}
}

void SerialUVScroll::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	loopCount_ = data["loopCount_"];
	cols_ = data["cols_"];
	rows_ = data["rows_"];
	rowsUse_ = data["rowsUse_"];

	textureSize_.x = data["textureSize_.x"];
	textureSize_.y = data["textureSize_.y"];
	cellSize_.x = data["cellSize_.x"];
	cellSize_.y = data["cellSize_.y"];
	origin_.x = data["origin_.x"];
	origin_.y = data["origin_.y"];
	gapSize_.x = data["gapSize_.x"];
	gapSize_.y = data["gapSize_.y"];

	startCorner_ = EnumAdapter<StartCorner>::FromString(data["startCorner_"]).value();
	horizontal_ = EnumAdapter<HorizontalDirection>::FromString(data["horizontal_"]).value();
	vertical_ = EnumAdapter<VerticalDirection>::FromString(data["vertical_"]).value();
	traverse_ = EnumAdapter<Traverse>::FromString(data["traverse_"]).value();
	horizontalCounts_ = data["horizontalCounts_"].get<std::vector<int>>();
}

void SerialUVScroll::ToJson(Json& data) const {

	data["loopCount_"] = loopCount_;
	data["cols_"] = cols_;
	data["rows_"] = rows_;
	data["rowsUse_"] = rowsUse_;

	data["textureSize_.x"] = textureSize_.x;
	data["textureSize_.y"] = textureSize_.y;
	data["cellSize_.x"] = cellSize_.x;
	data["cellSize_.y"] = cellSize_.y;
	data["origin_.x"] = origin_.x;
	data["origin_.y"] = origin_.y;
	data["gapSize_.x"] = gapSize_.x;
	data["gapSize_.y"] = gapSize_.y;

	data["startCorner_"] = EnumAdapter<StartCorner>::ToString(startCorner_);
	data["horizontal_"] = EnumAdapter<HorizontalDirection>::ToString(horizontal_);
	data["vertical_"] = EnumAdapter<VerticalDirection>::ToString(vertical_);
	data["traverse_"] = EnumAdapter<Traverse>::ToString(traverse_);
	data["horizontalCounts_"] = horizontalCounts_;
}