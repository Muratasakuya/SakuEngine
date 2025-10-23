#pragma once

//============================================================================
//	AssetEditorPayloadData
//	AssetEditor間のドラッグ&ドロップで使用するペイロード定義と種別
//	ImGuiのSetDragDropPayloadで受け渡す識別子とデータ構造を提供する
//============================================================================

static constexpr const char* kDragPayloadId = "ASSET_PATH";

// ファイルの種類
enum class PendingType {

	None,
	Texture,
	Model
};
// ドラッグ開始時に渡す簡易ペイロード(種類と名前のみ)
struct DragPayload {

	PendingType type; // タイプ
	char name[260];   // 名前
};