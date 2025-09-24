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
//============================================================================
class ModelLoader {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ModelLoader() = default;
	~ModelLoader() = default;

	void Init(TextureManager* textureManager);

	// 読み込み処理
	void LoadSynch(const std::string& modelName);
	void Load(const std::string& modelName);
	void RequestLoadAsync(const std::string& modelName);

	bool Search(const std::string& modelName);
	void WaitAll();
	void ReportUsage(bool listAll) const;

	//--------- accessor -----------------------------------------------------

	const ModelData& GetModelData(const std::string& modelName) const;
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

	ModelData LoadModelFile(const std::string& filePath);
	Node ReadNode(aiNode* node);

	// 非同期処理
	void LoadAsync(std::string modelName);
};