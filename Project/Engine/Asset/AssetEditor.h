#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/AssetEditorPayloadData.h>
#include <Engine/MathLib/Vector2.h>

// directX
#include <d3d12.h>
// c++
#include <string>
#include <vector>
#include <memory>
#include <stack>
#include <unordered_map>
#include <filesystem>
// imgui
#include <imgui.h>
// front
class Asset;

//============================================================================
//	AssetEditor class
//	Assetsディレクトリをツリー/グリッド表示し、プレビュー、D&D、ロード操作を提供
//	レイアウト編集、コンテキストボタン、エディタ用アイコン管理を担当するUIクラス
//============================================================================
class AssetEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	// 参照するAssetを受け取り、アイコン取得/ディレクトリツリー構築/設定適用を行う
	void Init(Asset* asset);

	// フォルダサイズや余白などUIレイアウトの編集UIを表示する
	void EditLayout();
	// メインのエディタウィンドウを描画する
	void ImGui();

	//--------- accessor -----------------------------------------------------

	// シングルトンインスタンスを取得/破棄する
	static AssetEditor* GetInstance();
	static void Finalize();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// ディレクトリツリーを構成するノード情報(表示名/パス/子ノード/状態フラグ)
	struct DirectoryNode {

		std::string name;                                     // フォルダ名
		std::filesystem::path path;                           // 相対パス
		std::vector<std::unique_ptr<DirectoryNode>> children; // サブフォルダ
		bool opened;                                          // UIの開閉フラグ
		bool isDirectory;
	};

	//--------- variables ----------------------------------------------------

	// json
	const std::string baseJsonPath_ = "AssetEditor/";

	static AssetEditor* instance_;

	Asset* asset_;
	std::unique_ptr<DirectoryNode> root_;
	DirectoryNode* current_;               // 現在表示中のノード
	std::vector<DirectoryNode*> navStack_; // 戻る用スタック

	D3D12_GPU_DESCRIPTOR_HANDLE folderIcon_; // デフォルトフォルダ
	D3D12_GPU_DESCRIPTOR_HANDLE fileIcon_;   // json、その他ファイル
	D3D12_GPU_DESCRIPTOR_HANDLE modelIcon_;  // obj、gltf
	D3D12_GPU_DESCRIPTOR_HANDLE textureIcon_;// 未ロードテクスチャ

	std::filesystem::path pendingPath_;
	PendingType pendingType_;
	bool showLoadButton_;
	ImVec2 overlayPos_;

	// parameter
	float folderSize_;          // 各階層のfolderの画像サイズ
	float folderSpacing_;       // 各folder間の幅、縦横同じ
	float folderNameSpancing_;  // folderと名前の間
	float charScale_;           // 文字のサイズスケール
	float chidNameOffset_;      // 文字の左側オフセット
	Vector2 folderOffset_;      // 左上のオフセット
	Vector2 loadOverlayOffset_; // loadのオフセット

	//--------- functions ----------------------------------------------------

	// 設定ファイルを適用する/保存する
	void ApplyJson();
	void SaveJson();

	// ルートから再帰走査してツリーを再構築する(runTime=trueで初期化からやり直し)
	void BuildDirectoryTree(bool runTime);

	// パンくずと再構築ボタンを含むヘッダを描画する
	void DrawHeader();
	// 現在ディレクトリの子要素をアイコンのグリッドで描画し、入力を処理する
	void DrawFolderGrid();
	// 右クリック位置にロード候補の小窓を表示し、即時ロードを提供する
	void DrawLoadOverlay();

	// ヘルパ: 拡張子判定/アイコン選択など
	bool IsTextureFile(const std::filesystem::path& p) const;
	bool IsModelFile(const std::filesystem::path& path) const;
	bool IsJsonFile(const std::filesystem::path& path) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetIconForEntry(const DirectoryNode& entry) const;

	AssetEditor() = default;
	~AssetEditor() = default;
	AssetEditor(const AssetEditor&) = delete;
	AssetEditor& operator=(const AssetEditor&) = delete;
};