#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxObject/DxUploadCommand.h>
#include <Engine/Asset/Async/AssetLoadWorker.h>

// directX
#include <Externals/DirectXTex/DirectXTex.h>
#include <Externals/DirectXTex/d3dx12.h>
#include <DirectXMath.h>
#include <directxpackedvector.h>
// c++
#include <string>
#include <istream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
// front
class DxCommand;
class SRVDescriptor;

//============================================================================
//	TextureManager class
//	テクスチャの探索/読み込み、ミップ生成、GPUリソース作成と転送、SRV割当て、
//	使用状況の収集とレポートを行うアセット管理クラス
//============================================================================
class TextureManager {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	TextureManager() = default;
	~TextureManager() = default;

	// 必要なデバイス/コマンド/ディスクリプタを受け取り、ワーカーとアップロード系を初期化
	void Init(ID3D12Device* device, DxCommand* dxCommand, SRVDescriptor* srvDescriptor);

	// 同期ロード：即時読み込み→ミップ生成→GPU転送→SRV作成まで実行
	void LoadSynch(const std::string& textureName);
	// 非同期ロードを要求し、完了までブロックして待機
	void Load(const std::string& textureName);
	// 非同期ロードをキューに積む(重複/既存チェックあり)
	void RequestLoadAsync(const std::string& textureName);

	// ロード済みかの有無を確認
	bool Search(const std::string& textureName);
	// キューが空になるまで待機
	void WaitAll();
	// 未使用/未ロード(ディスクのみ)を集計しログ出力listAllで一覧
	void ReportUsage(bool listAll) const;

	//--------- accessor -----------------------------------------------------

	// 描画用GPUハンドルを取得(存在しなければASSERT/ログ)、取得時に使用フラグを更新
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle(const std::string textureName) const;
	// SRVテーブル上のインデックスを取得(存在しなければASSERT/ログ)
	uint32_t GetTextureGPUIndex(const std::string& textureName) const;
	// 読み込んだメタデータ(サイズ/ミップ/フォーマット等)を取得
	const DirectX::TexMetadata& GetMetaData(const std::string textureName) const;

	// 階層(フォルダ階層)文字列一覧を取得
	std::vector<std::string> GetTextureHierarchies() const;
	// テクスチャ識別子一覧を取得
	const std::vector<std::string>& GetTextureKeys() const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structures ----------------------------------------------------

	// 1枚のテクスチャに関するGPUリソースとメタ情報を保持する内部データ
	struct TextureData {

		ComPtr<ID3D12Resource> resource;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		DirectX::TexMetadata metadata;
		uint32_t srvIndex;
		std::string hierarchy;

		// 使用されたかどうか
		mutable bool isUse = false;
	};

	//--------- variables ----------------------------------------------------

	ID3D12Device* device_;
	DxCommand* dxCommand_;
	SRVDescriptor* srvDescriptor_;

	std::string baseDirectoryPath_;

	std::unordered_map<std::string, TextureData> textures_;

	mutable std::vector<std::string> textureKeysCache_;
	mutable bool isCacheValid_;

	// 非同期処理
	std::unique_ptr<DxUploadCommand> dxUploadCommand_;

	AssetLoadWorker<std::string> loadWorker_;
	std::mutex gpuMutex_;

	//--------- functions ----------------------------------------------------

	// 拡張子や法線判定に応じて読み込み、適切なミップを生成して返す
	DirectX::ScratchImage GenerateMipMaps(const std::filesystem::path& filePath, DirectX::TexMetadata& outMeta);

	// 非同期ジョブ本体：指定テクスチャを読み込み→GPUへアップロード→登録
	void LoadAsync(std::string name);
	// リソース作成・UploadCmdでの転送・SRV生成までをまとめて行う
	void CreateAndUpload(const std::string& identifier,
		const DirectX::ScratchImage& mipImages, const DirectX::TexMetadata& meta);
};