#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/TextureManager.h>
#include <Engine/Asset/ModelLoader.h>
#include <Engine/Asset/AnimationManager.h>
#include <Engine/Asset/AssetLoadType.h>
#include <Engine/Scene/Methods/IScene.h>

// c++
#include <memory>
#include <mutex>
#include <functional>
#include <deque>
#include <cctype>

//============================================================================
//	Asset class
//============================================================================
class Asset {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Asset() = default;;
	~Asset() = default;

	// 初期化
	void Init(ID3D12Device* device, DxCommand* dxCommand, SRVDescriptor* srvDescriptor);
	// ログ出力
	void ReportUsage(bool listAll = false) const;

	//--------- loading ------------------------------------------------------

	// シーンアセットファイルの読み込み
	void LoadSceneAsync(Scene scene, AssetLoadType loadType);
	// 非同期読み込みの更新
	void PumpAsyncLoads();

	void LoadTexture(const std::string& textureName, AssetLoadType loadType);
	void LoadModel(const std::string& modelName, AssetLoadType loadType);
	void LoadAnimation(const std::string& animationName, const std::string& modelName);
	
	//--------- accessor -----------------------------------------------------

	// 非同期読み込み処理が終わっているかどうか
	bool IsScenePreloadFinished(Scene scene) const;
	float GetScenePreloadProgress(Scene scene) const;

	//--------- textures -----------------------------------------------------

	// 描画に必要なデータ
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle(const std::string textureName) const;
	uint32_t GetTextureGPUIndex(const std::string& textureName) const;
	const DirectX::TexMetadata& GetMetaData(const std::string textureName) const;

	// エディターで使用するデータ
	std::vector<std::string> GetTextureHierarchies() const;
	const std::vector<std::string>& GetTextureKeys() const;
	bool SearchTexture(const std::string& textureName);

	//---------- models ------------------------------------------------------

	// 描画に必要なデータ
	const ModelData& GetModelData(const std::string& modelName) const;
	const std::vector<std::string>& GetModelKeys() const;
	const std::vector<std::string>& GetPreloadModels(Scene scene) const;
	// エディターで使用するデータ
	bool SearchModel(const std::string& modelName);

	//--------- animation ----------------------------------------------------

	// 描画に必要なデータ
	const AnimationData& GetAnimationData(const std::string& animationName) const;
	const Skeleton& GetSkeletonData(const std::string& animationName) const;
	const SkinCluster& GetSkinClusterData(const std::string& animationName) const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 読み込み進捗度
	struct ScenePreload {

		Scene scene;           // シーンの種類
		uint32_t total = 0;    // キュー投入総数

		std::vector<std::string> textures;
		std::vector<std::string> models;
		std::vector<std::pair<std::string, std::string>> animations;
	};

	//--------- variables ----------------------------------------------------

	// assetを管理する
	std::unique_ptr<TextureManager> textureManager_;
	std::unique_ptr<ModelLoader> modelLoader_;
	std::unique_ptr<AnimationManager> animationManager_;

	// 1フレームで処理される読み込みスレッド数
	const uint32_t maxCountPerFrame_ = 1;

	std::mutex asyncMutex_;
	std::deque<std::function<void()>> pendingLoads_;  // 実行待ちタスク
	std::unordered_map<Scene, ScenePreload> preload_; // シーン別進捗度

	//--------- functions ----------------------------------------------------

	// helper
	std::vector<std::function<void()>> SetTask(const Json& data, AssetLoadType loadType);
};