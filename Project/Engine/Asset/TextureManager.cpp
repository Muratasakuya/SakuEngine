#include "TextureManager.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Core/Debug/SpdLogger.h>
#include <Engine/Core/Graphics/DxObject/DxCommand.h>
#include <Engine/Core/Graphics/Descriptors/SRVDescriptor.h>
#include <Engine/Asset/Filesystem.h>
#include <Engine/Utility/Helper/Algorithm.h>

//============================================================================
//	TextureManager classMethods
//============================================================================

void TextureManager::Init(ID3D12Device* device, DxCommand* dxCommand,
	SRVDescriptor* srvDescriptor) {

	device_ = nullptr;
	device_ = device;

	dxCommand_ = nullptr;
	dxCommand_ = dxCommand;

	srvDescriptor_ = nullptr;
	srvDescriptor_ = srvDescriptor;

	baseDirectoryPath_ = "./Assets/Textures/";
	isCacheValid_ = false;

	// 転送用
	dxUploadCommand_ = std::make_unique<DxUploadCommand>();
	dxUploadCommand_->Create(device_);

	// ワーカースレッド起動
	loadWorker_.Start([this](std::string&& name) {
		this->LoadAsync(std::move(name)); });
}

void TextureManager::LoadSynch(const std::string& textureName) {

	// すでにロード済みなら何もしない
	{
		std::scoped_lock lk(gpuMutex_);
		if (textures_.contains(textureName)) { 
			return;
		}
	}

	// 読み込み開始
	SpdLogger::Log("[Texture][Begin] " + textureName);

	std::filesystem::path path;
	// 見つからなければ処理しない
	if (!Filesystem::FindByStem(baseDirectoryPath_, textureName, { ".png",".jpg",".dds" }, path)) {
		SpdLogger::Log("[Texture][Missing] " + textureName);
		return;
	}
	// 識別名取得
	const std::string identifier = path.stem().string();

	// ミップマップの作成
	DirectX::TexMetadata meta{};
	auto mip = GenerateMipMaps(path, meta);
	// リソース作成してGPUに転送
	CreateAndUpload(identifier, mip, meta);

	// 階層を設定
	{
		std::scoped_lock lock(gpuMutex_);

		std::filesystem::path relative = std::filesystem::relative(path, baseDirectoryPath_);
		relative.replace_extension();
		textures_[identifier].hierarchy = relative.generic_string();
	}

	SpdLogger::Log("[Texture][SyncLoad][End] " + identifier);
}

void TextureManager::Load(const std::string& textureName) {

	RequestLoadAsync(textureName);
	for (;;) {
		{
			std::scoped_lock lk(gpuMutex_);
			if (textures_.contains(textureName)) {
				break;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void TextureManager::RequestLoadAsync(const std::string& textureName) {

	// 既にロード済みなら何もしない
	{
		std::scoped_lock lk(gpuMutex_);
		if (textures_.contains(textureName)) {
			return;
		}
	}
	// 重複チェック後にキューを追加
	auto& queue = loadWorker_.RefAsyncQueue();
	if (queue.IsClearCondition([&](const std::string& j) { return j == textureName; })) {
		return;
	}
	queue.AddQueue(textureName);
	SpdLogger::Log("[Texture][Enqueue] " + textureName);
}

void TextureManager::WaitAll() {

	for (;;) {
		if (loadWorker_.GetAsyncQueue().IsEmpty()) {

			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void TextureManager::LoadAsync(std::string name) {

	// 重複読み込みを行わないようにチェック
	{
		std::scoped_lock lock(gpuMutex_);
		if (textures_.contains(name)) {
			return;
		}
	}

	// 読み込み開始
	SpdLogger::Log("[Texture][Begin] " + name);

	std::filesystem::path path;
	// 見つからなければ処理しない
	if (!Filesystem::FindByStem(baseDirectoryPath_, name, { ".png",".jpg",".dds" }, path)) {
		SpdLogger::Log("[Texture][Missing] " + name);
		return;
	}
	// 識別名取得
	const std::string identifier = path.stem().string();

	// ミップマップの作成
	DirectX::TexMetadata meta{};
	auto mip = GenerateMipMaps(path, meta);
	// リソース作成してGPUに転送
	CreateAndUpload(identifier, mip, meta);

	// 階層を設定
	{
		std::scoped_lock lock(gpuMutex_);

		std::filesystem::path relative = std::filesystem::relative(path, baseDirectoryPath_);
		relative.replace_extension();
		textures_[identifier].hierarchy = relative.generic_string();
	}
}

DirectX::ScratchImage TextureManager::GenerateMipMaps(const std::filesystem::path& filePath, DirectX::TexMetadata& outMeta) {

	// テクスチャファイルを呼んでプログラムを扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = Algorithm::ConvertString(filePath.string());

	HRESULT hr = S_OK;
	// dds拡張子かどうかで分岐させる
	if (filePathW.ends_with(L".dds")) {

		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
		assert(SUCCEEDED(hr));
	} else {

		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB | DirectX::WIC_FLAGS_DEFAULT_SRGB, nullptr, image);
		assert(SUCCEEDED(hr));
	}

	DirectX::ScratchImage mipImages{};
	// 圧縮フォーマットかどうかチェックして分岐させる
	if (DirectX::IsCompressed(image.GetMetadata().format)) {

		// 圧縮フォーマットだったらそのまま使用する
		mipImages = std::move(image);
	} else {

		// ミップマップの作成 → 元画像よりも小さなテクスチャ群
		hr = DirectX::GenerateMipMaps(
			image.GetImages(), image.GetImageCount(), image.GetMetadata(),
			DirectX::TEX_FILTER_SRGB, 4, mipImages);
		assert(SUCCEEDED(hr));
	}
	// メタデータ取得
	outMeta = mipImages.GetMetadata();

	// ミップマップ付きのデータを返す
	return mipImages;
}

void TextureManager::CreateAndUpload(const std::string& identifier,
	const DirectX::ScratchImage& mipImages, const DirectX::TexMetadata& meta) {

	TextureData& texture = textures_[identifier];
	texture.metadata = meta;
	texture.isUse = false;

	// GPUリソース作成
	{
		D3D12_RESOURCE_DESC desc{};
		desc.Width = UINT(meta.width);
		desc.Height = UINT(meta.height);
		desc.MipLevels = UINT16(meta.mipLevels);
		desc.DepthOrArraySize = UINT16(meta.arraySize);
		desc.Format = meta.format;
		desc.SampleDesc.Count = 1;
		desc.Dimension = D3D12_RESOURCE_DIMENSION(meta.dimension);

		D3D12_HEAP_PROPERTIES heap{};
		heap.Type = D3D12_HEAP_TYPE_DEFAULT;

		HRESULT hr = device_->CreateCommittedResource(
			&heap, D3D12_HEAP_FLAG_NONE, &desc,
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture.resource));
		assert(SUCCEEDED(hr));
	}

	// サブリソース
	std::vector<D3D12_SUBRESOURCE_DATA> subResources;
	DirectX::PrepareUpload(device_, mipImages.GetImages(), mipImages.GetImageCount(), meta, subResources);

	// 中間バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
	{
		uint64_t size = GetRequiredIntermediateSize(texture.resource.Get(), 0, static_cast<UINT>(subResources.size()));

		D3D12_HEAP_PROPERTIES heap{};
		heap.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC buf{};
		buf.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		buf.Width = size; buf.Height = 1; buf.DepthOrArraySize = 1;
		buf.MipLevels = 1; buf.SampleDesc.Count = 1; buf.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		HRESULT hr = device_->CreateCommittedResource(
			&heap, D3D12_HEAP_FLAG_NONE, &buf, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
		assert(SUCCEEDED(hr));
	}

	ID3D12GraphicsCommandList* commandList = dxUploadCommand_->GetCommandList();
	UpdateSubresources(commandList, texture.resource.Get(), uploadBuffer.Get(),
		0, 0, static_cast<UINT>(subResources.size()), subResources.data());

	// COPY_DEST -> GENERIC_READ
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = texture.resource.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(1, &barrier);

	// GPUに転送する
	dxUploadCommand_->ExecuteCommands();
	SpdLogger::Log(std::string("[Texture][Upload->GPU][End]") + identifier);

	std::scoped_lock lk(gpuMutex_);

	// SRV作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = meta.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	// CubeMapかどうかで分岐
	if (meta.IsCubemap()) {

		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = UINT_MAX;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	} else {

		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = (UINT)meta.mipLevels;
	}
	srvDescriptor_->CreateSRV(texture.srvIndex, texture.resource.Get(), srvDesc);
	texture.gpuHandle = srvDescriptor_->GetGPUHandle(texture.srvIndex);

	std::wstring resourceName = std::wstring(identifier.begin(), identifier.end());
	// debug用にGPUResourceにtextureの名前を設定する
	texture.resource->SetName(resourceName.c_str());

	isCacheValid_ = false;
}

bool TextureManager::Search(const std::string& textureName) {

	return Algorithm::Find(textures_, textureName);
}

const D3D12_GPU_DESCRIPTOR_HANDLE& TextureManager::GetGPUHandle(const std::string textureName) const {

	auto it = textures_.find(textureName);
	if (it == textures_.end()) {

		LOG_WARN("not found texture", textureName);
		ASSERT(FALSE, "not found texture" + textureName);
	}

	// 使用された
	it->second.isUse = true;
	return it->second.gpuHandle;
}

uint32_t TextureManager::GetTextureGPUIndex(const std::string& textureName) const {

	auto it = textures_.find(textureName);
	if (it == textures_.end()) {

		LOG_WARN("not found texture", textureName);
		ASSERT(FALSE, "not found texture" + textureName);
	}

	// 使用された
	it->second.isUse = true;
	return it->second.srvIndex;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string textureName) const {

	auto it = textures_.find(textureName);
	if (it == textures_.end()) {

		LOG_WARN("not found texture", textureName);
		ASSERT(FALSE, "not found texture" + textureName);
	}

	// 使用された
	it->second.isUse = true;
	return it->second.metadata;
}

std::vector<std::string> TextureManager::GetTextureHierarchies() const {

	std::vector<std::string> keys;
	for (const auto& [id, tex] : textures_) {

		keys.push_back(tex.hierarchy);
	}
	return keys;
}

const std::vector<std::string>& TextureManager::GetTextureKeys() const {

	if (!isCacheValid_) {

		textureKeysCache_.clear();
		textureKeysCache_.reserve(textures_.size());
		for (const auto& pair : textures_) {

			textureKeysCache_.emplace_back(pair.first);
		}
		isCacheValid_ = true;
	}
	return textureKeysCache_;
}

void TextureManager::ReportUsage(bool listAll) const {

	// ロード済みだが未使用の場合のログ出力
	std::vector<std::string> unused;
	unused.reserve(textures_.size());
	for (const auto& [name, tex] : textures_) {
		if (!tex.isUse) {

			unused.emplace_back(name);
		}
	}

	if (unused.empty()) {

		LOG_ASSET_INFO("[Texture] Unused: 0");
	} else {

		LOG_ASSET_INFO("[Texture] Unused: {}", unused.size());
		if (listAll) {
			for (auto& n : unused) {

				LOG_ASSET_INFO("  - {}", n);
			}
		}
	}

	// フォルダ内にあるにも関わらず未使用
	std::unordered_set<std::string> onDisk;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(baseDirectoryPath_)) {
		if (!entry.is_regular_file()) {
			continue;
		}
		const auto ext = entry.path().extension().string();
		if (ext == ".png" || ext == ".jpg" || ext == ".dds" || ext == ".cube") {

			onDisk.insert(entry.path().stem().string());
		}
	}

	std::unordered_set<std::string> loaded;
	loaded.reserve(textures_.size());
	for (const auto& [name, _] : textures_) {

		loaded.insert(name);
	}

	std::vector<std::string> notLoaded;
	notLoaded.reserve(onDisk.size());
	for (auto& stem : onDisk) {
		if (!loaded.contains(stem)) {

			notLoaded.emplace_back(stem);
		}
	}

	if (notLoaded.empty()) {

		LOG_ASSET_INFO("[Texture] NotLoaded(on disk only): 0");
	} else {

		LOG_ASSET_INFO("[Texture] NotLoaded(on disk only): {}", notLoaded.size());
		if (listAll) {
			for (auto& n : notLoaded) {

				LOG_ASSET_INFO("  - {}", n);
			}
		}
	}
}