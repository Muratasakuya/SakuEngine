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
//	テクスチャ/モデル/アニメーションの集中管理を行うアセットハブ
//	同期/非同期ロード、進捗管理、描画用ハンドル提供、エディタ支援APIを担う。
//============================================================================
class Asset {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Asset() = default;;
	~Asset() = default;

	// 必要なマネージャを生成し、各種デバイス/ディスクリプタで初期化する
	void Init(ID3D12Device* device, DxCommand* dxCommand, SRVDescriptor* srvDescriptor);
	// 読み込み状況をログ出力する(listAll=true で未使用資産も一覧)
	void ReportUsage(bool listAll = false) const;

	//--------- loading ------------------------------------------------------

	// シーンに紐づくアセット定義(json)を読み取り、同期または非同期でロードする
	void LoadSceneAsync(Scene scene, AssetLoadType loadType);
	//  キューされた非同期タスクを1フレームあたりの上限数まで実行する
	void PumpAsyncLoads();

	// 個別資産のロード要求(同期/非同期)
	void LoadTexture(const std::string& textureName, AssetLoadType loadType);
	void LoadModel(const std::string& modelName, AssetLoadType loadType);
	// アニメーションをモデルに対して読み込む(同期)
	void LoadAnimation(const std::string& animationName, const std::string& modelName);
	
	//--------- accessor -----------------------------------------------------

	//  シーンのプリロードが完了しているかを返す
	bool IsScenePreloadFinished(Scene scene) const;
	// シーンのプリロード進捗を0.0f〜1.0fで返す
	float GetScenePreloadProgress(Scene scene) const;

	//--------- textures -----------------------------------------------------

	// 描画に必要なGPUハンドル/インデックス/メタデータを取得する
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle(const std::string textureName) const;
	uint32_t GetTextureGPUIndex(const std::string& textureName) const;
	const DirectX::TexMetadata& GetMetaData(const std::string textureName) const;

	// エディタ向けの補助情報を取得/検索する
	std::vector<std::string> GetTextureHierarchies() const;
	const std::vector<std::string>& GetTextureKeys() const;
	bool SearchTexture(const std::string& textureName);

	//---------- models ------------------------------------------------------

	// 描画用のモデルデータやキー一覧を取得する
	const ModelData& GetModelData(const std::string& modelName) const;
	const std::vector<std::string>& GetModelKeys() const;
	// プリロード対象モデル名リストを取得する
	const std::vector<std::string>& GetPreloadModels(Scene scene) const;
	// モデルが読み込み済みかを確認する
	bool SearchModel(const std::string& modelName);

	//--------- animation ----------------------------------------------------

	// 描画に必要なアニメーション/スケルトン/スキンクラスタ情報を取得する
	const AnimationData& GetAnimationData(const std::string& animationName) const;
	const Skeleton& GetSkeletonData(const std::string& animationName) const;
	const SkinCluster& GetSkinClusterData(const std::string& animationName) const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// シーンのプリロード対象一覧と合計数を保持し、進捗算出に用いるコンテナ
	struct ScenePreload {

		Scene scene;        // シーンの種類
		uint32_t total = 0; // キュー投入総数

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
	// jsonからロードタスク群を構築し、同期/非同期方針に合わせた関数オブジェクトを返す
	std::vector<std::function<void()>> SetTask(const Json& data, AssetLoadType loadType);
};