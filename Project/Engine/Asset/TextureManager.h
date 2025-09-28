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
//============================================================================
class TextureManager {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	TextureManager() = default;
	~TextureManager() = default;

	void Init(ID3D12Device* device, DxCommand* dxCommand, SRVDescriptor* srvDescriptor);

	// 読み込み処理
	void LoadSynch(const std::string& textureName);
	void Load(const std::string& textureName);
	void RequestLoadAsync(const std::string& textureName);

	bool Search(const std::string& textureName);
	void WaitAll();
	void ReportUsage(bool listAll) const;

	//--------- accessor -----------------------------------------------------

	// 描画に使用するデータ
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle(const std::string textureName) const;
	uint32_t GetTextureGPUIndex(const std::string& textureName) const;
	const DirectX::TexMetadata& GetMetaData(const std::string textureName) const;

	std::vector<std::string> GetTextureHierarchies() const;
	const std::vector<std::string>& GetTextureKeys() const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structures ----------------------------------------------------

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

	// helper
	DirectX::ScratchImage GenerateMipMaps(const std::filesystem::path& filePath, DirectX::TexMetadata& outMeta);

	// 非同期処理
	void LoadAsync(std::string name);
	void CreateAndUpload(const std::string& identifier,
		const DirectX::ScratchImage& mipImages, const DirectX::TexMetadata& meta);
};