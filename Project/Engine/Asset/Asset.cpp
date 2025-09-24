#include "Asset.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Core/Debug/SpdLogger.h>
#include <Engine/Utility/JsonAdapter.h>
#include <Engine/Utility/EnumAdapter.h>

//============================================================================
//	Asset classMethods
//============================================================================

void Asset::Init(ID3D12Device* device, DxCommand* dxCommand, SRVDescriptor* srvDescriptor) {

	textureManager_ = std::make_unique<TextureManager>();
	textureManager_->Init(device, dxCommand, srvDescriptor);

	modelLoader_ = std::make_unique<ModelLoader>();
	modelLoader_->Init(textureManager_.get());

	animationManager_ = std::make_unique<AnimationManager>();
	animationManager_->Init(device, srvDescriptor, modelLoader_.get());
}

void Asset::ReportUsage(bool listAll) const {

	// 全てのアセットファイルのログ出力
	textureManager_->ReportUsage(listAll);
	modelLoader_->ReportUsage(listAll);
}

void Asset::PumpAsyncLoads() {

	// 最大数分の件数を毎フレーム実行
	size_t executed = 0;
	for (;;) {

		std::function<void()> task;
		{
			std::scoped_lock lock(asyncMutex_);
			if (pendingLoads_.empty()) {
				break;
			}
			task = std::move(pendingLoads_.front());
			pendingLoads_.pop_front();
		}
		task();
		executed++;
		if (maxCountPerFrame_ <= executed) {
			break;
		}
	}
}

bool Asset::IsScenePreloadFinished(Scene scene) const {

	// 読み込みが終了しているか
	return 1.0f <= GetScenePreloadProgress(scene);
}

float Asset::GetScenePreloadProgress(Scene scene) const {

	// 読み込み進捗度(0.0f ~ 1.0f)
	auto it = preload_.find(scene);
	if (it == preload_.end()) {
		return 0.0f;
	}
	const auto& load = it->second;
	if (load.total == 0) {
		return 1.0f;
	}
	// 読み込み済み
	uint32_t ready = 0;

	// テクスチャ
	for (const auto& t : load.textures) {
		if (textureManager_->Search(t)) {

			++ready;
		}
	}
	// モデル
	for (const auto& m : load.models) {
		if (modelLoader_->Search(m)) {

			++ready;
		}
	}

	for (const auto& [anim, model] : load.animations) {
		if (modelLoader_->Search(model)) {

			++ready;
		}
	}
	float progress = static_cast<float>(ready) / static_cast<float>(load.total);
	return std::clamp(progress, 0.0f, 1.0f);
}

std::vector<std::function<void()>> Asset::SetTask(const Json& data, AssetLoadType loadType) {

	std::vector<std::function<void()>> tasks{};
	// texture
	{
		if (data.contains("Textures") && data["Textures"].is_array()) {
			for (auto& name : data["Textures"]) {

				std::string texture = name.get<std::string>();
				if (loadType == AssetLoadType::Synch) {

					tasks.emplace_back([this, texture]() { this->textureManager_->LoadSynch(texture); });
				} else if (loadType == AssetLoadType::Async) {

					tasks.emplace_back([this, texture]() { this->textureManager_->RequestLoadAsync(texture); });
				}
			}
		}
	}
	// model
	{
		if (data.contains("Models") && data["Models"].is_array()) {
			for (auto& name : data["Models"]) {

				std::string model = name.get<std::string>();
				if (loadType == AssetLoadType::Synch) {

					tasks.emplace_back([this, model]() { this->modelLoader_->LoadSynch(model); });
				} else if (loadType == AssetLoadType::Async) {

					tasks.emplace_back([this, model]() { this->modelLoader_->RequestLoadAsync(model); });
				}
			}
		}
	}
	// animation
	{
		if (data.contains("Animations") && data["Animations"].is_array()) {
			for (auto& a : data["Animations"]) {

				std::string model = a["model"].get<std::string>();
				std::string animation = a["animation"].get<std::string>();
				tasks.emplace_back([this, animation, model]() { this->animationManager_->RequestLoadAsync(animation, model); });
			}
		}
	}
	return tasks;
}

void Asset::LoadSceneAsync(Scene scene, AssetLoadType loadType) {

	// Sceneの種類に応じてファイル名を決定する
	std::string sceneName = EnumAdapter<Scene>::ToString(scene);
	// 1文字目を小文字に変換する
	sceneName[0] = static_cast<char>(std::tolower(sceneName[0]));
	std::string fileName = "Scene/Asset/" + sceneName + "Scene.json";

	// 読み込み処理
	Json data{};
	if (!JsonAdapter::LoadCheck(fileName, data)) {
		// エラー
		LOG_WARN("sceneFile not found → {}", fileName);
		ASSERT(FALSE, "sceneFile not found: " + fileName);
	}

	//	タスク処理設定
	std::vector<std::function<void()>> tasks = SetTask(data, loadType);

	// 同期、非同期読み込み処理振り分け
	auto& info = preload_[scene];
	info.scene = scene;

	// 必要なデータの名前を取得
	if (data.contains("Textures") && data["Textures"].is_array()) {
		for (const auto& texture : data["Textures"]) {

			info.textures.emplace_back(texture.get<std::string>());
		}
	}
	if (data.contains("Models") && data["Models"].is_array()) {
		for (const auto& model : data["Models"]) {

			info.models.emplace_back(model.get<std::string>());
		}
	}
	if (data.contains("Animations") && data["Animations"].is_array()) {
		for (const auto& animation : data["Animations"]) {

			info.animations.emplace_back(animation["animation"].get<std::string>(),
				animation["model"].get<std::string>());
		}
	}
	// リソース合計数
	info.total = static_cast<uint32_t>(info.textures.size() + info.models.size() + info.animations.size());

	// 同期読み込み処理
	if (loadType == AssetLoadType::Synch) {
		for (auto& task : tasks) {

			task();
		}
		return;
	}
	{
		// 非同期読み込み処理
		std::scoped_lock lock(asyncMutex_);
		for (auto& task : tasks) {

			pendingLoads_.emplace_back([this, scene, task]() {task(); });
		}
	}
}

void Asset::LoadTexture(const std::string& textureName, AssetLoadType loadType) {

	if (loadType == AssetLoadType::Synch) {

		textureManager_->LoadSynch(textureName);
	} else if (loadType == AssetLoadType::Async) {

		textureManager_->Load(textureName);
	}
}

void Asset::LoadModel(const std::string& modelName, AssetLoadType loadType) {

	if (loadType == AssetLoadType::Synch) {

		modelLoader_->LoadSynch(modelName);
	} else if (loadType == AssetLoadType::Async) {

		modelLoader_->Load(modelName);
	}
}

void Asset::LoadAnimation(const std::string& animationName, const std::string& modelName) {
	animationManager_->Load(animationName, modelName);
}

const D3D12_GPU_DESCRIPTOR_HANDLE& Asset::GetGPUHandle(const std::string textureName) const {
	return textureManager_->GetGPUHandle(textureName);
}

uint32_t Asset::GetTextureGPUIndex(const std::string& textureName) const {
	return textureManager_->GetTextureGPUIndex(textureName);
}

const DirectX::TexMetadata& Asset::GetMetaData(const std::string textureName) const {
	return textureManager_->GetMetaData(textureName);
}

std::vector<std::string> Asset::GetTextureHierarchies() const {
	return textureManager_->GetTextureHierarchies();
}

const std::vector<std::string>& Asset::GetTextureKeys() const {
	return textureManager_->GetTextureKeys();
}

bool Asset::SearchTexture(const std::string& textureName) {
	return textureManager_->Search(textureName);
}

bool Asset::SearchModel(const std::string& modelName) {
	return modelLoader_->Search(modelName);
}

const ModelData& Asset::GetModelData(const std::string& modelName) const {
	return modelLoader_->GetModelData(modelName);
}

const std::vector<std::string>& Asset::GetModelKeys() const {
	return modelLoader_->GetModelKeys();
}

const std::vector<std::string>& Asset::GetPreloadModels(Scene scene) const {

	static const std::vector<std::string> kEmpty;
	auto it = preload_.find(scene);
	if (it == preload_.end()) {
		return kEmpty;
	}
	return it->second.models;
}

const AnimationData& Asset::GetAnimationData(const std::string& animationName) const {
	return animationManager_->GetAnimationData(animationName);
}

const Skeleton& Asset::GetSkeletonData(const std::string& animationName) const {
	return animationManager_->GetSkeletonData(animationName);
}

const SkinCluster& Asset::GetSkinClusterData(const std::string& animationName) const {
	return animationManager_->GetSkinClusterData(animationName);
}