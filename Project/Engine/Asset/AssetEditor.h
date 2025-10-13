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
//============================================================================
class AssetEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	void Init(Asset* asset);

	void EditLayout();
	void ImGui();

	//--------- accessor -----------------------------------------------------

	// singleton
	static AssetEditor* GetInstance();
	static void Finalize();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

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

	// json
	void ApplyJson();
	void SaveJson();

	// init
	void BuildDirectoryTree(bool runTime);

	// update
	void DrawHeader();
	void DrawFolderGrid();
	void DrawLoadOverlay();

	// helper
	bool IsTextureFile(const std::filesystem::path& p) const;
	bool IsModelFile(const std::filesystem::path& path) const;
	bool IsJsonFile(const std::filesystem::path& path) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetIconForEntry(const DirectoryNode& entry) const;

	AssetEditor() = default;
	~AssetEditor() = default;
	AssetEditor(const AssetEditor&) = delete;
	AssetEditor& operator=(const AssetEditor&) = delete;
};