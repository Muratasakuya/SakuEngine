#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/AssetStructure.h>
#include <Engine/Asset/Async/AssetLoadWorker.h>

// c++
#include <unordered_map>
#include <thread>
#include <mutex>
#include <deque>
// front
class TextureManager;

//============================================================================
//	ModelManager class
//	モデル資産の読み込み・解析(メッシュ/ボーン/階層)、非同期ロード管理、
//	読み込んだデータの提供と使用状況レポートを行う
//============================================================================
class ModelLoader {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ModelLoader() = default;
	~ModelLoader() = default;

	// 依存するTextureManagerを受け取り、基準パス設定とワーカー起動を行う
	void Init(TextureManager* textureManager);

	// 同期ロード：指定モデルを即時読み込みし、内部キャッシュへ登録
	void LoadSynch(const std::string& modelName);
	// 非同期ロードを要求し、完了までブロックして待機
	void Load(const std::string& modelName);
	// 非同期ロードをキューに積む(既存/重複を抑止)
	void RequestLoadAsync(const std::string& modelName);

	// 指定モデルが既にロード済みかを確認
	bool Search(const std::string& modelName);
	// 全非同期ジョブの完了を待機
	void WaitAll();
	// 未使用/未ロード(ディスクのみ)を集計しログ出力listAllで一覧
	void ReportUsage(bool listAll) const;

	//--------- accessor -----------------------------------------------------

	// モデルデータを取得(未登録時はASSERT/ログ)。取得時に使用フラグを更新
	const ModelData& GetModelData(const std::string& modelName) const;
	// ロード済みモデル名一覧を取得
	const std::vector<std::string>& GetModelKeys() const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	TextureManager* textureManager_;

	std::string baseDirectoryPath_;

	std::unordered_map<std::string, ModelData> models_;

	mutable std::vector<std::string> modelKeysCache_;
	mutable bool isCacheValid_;

	// 非同期処理
	AssetLoadWorker<std::string> loadWorker_;
	mutable std::mutex modelMutex_;

	//--------- functions ----------------------------------------------------

	// 読み込み、頂点/ボーン/マテリアルを解析してModelDataを生成
	ModelData LoadModelFile(const std::string& filePath);
	// AssimpノードからSRTと階層を再帰的に読み取り、Nodeを構築
	Node ReadNode(aiNode* node);

	// 非同期ジョブ本体：重複を避けつつ指定モデルをロードして登録
	void LoadAsync(std::string modelName);
};