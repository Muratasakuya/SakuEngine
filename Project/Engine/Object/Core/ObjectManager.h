#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectPoolManager.h>
#include <Engine/Object/System/Manager/SystemManager.h>

// directX
#include <d3d12.h>
// c++
#include <optional>
#include <string>
#include <unordered_set>
// front
class Asset;
class DxCommand;

//============================================================================
//	ObjectManager class
//	オブジェクト生成/破棄と各データ管理、各System連携を統括する
//============================================================================
class ObjectManager {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ObjectManager() = default;
	~ObjectManager() = default;

	// デバイス/アセット/コマンドを受け、各Systemとプールを初期化する
	void Init(ID3D12Device* device, Asset* asset, DxCommand* dxCommand);

	// 各Systemのデータ更新を実行する
	void UpdateData();
	// 各Systemのバッファ更新を実行する
	void UpdateBuffer();

	//---------- objects -----------------------------------------------------

	// object追加
	// helper
	// 3Dオブジェクト
	uint32_t CreateObjects(const std::string& modelName, const std::string& name,
		const std::string& groupName, const std::optional<std::string>& animationName = std::nullopt);
	// スカイボックス
	uint32_t CreateSkybox(const std::string& textureName);
	// エフェクト
	uint32_t CreateEffect(const std::string& name, const std::string& groupName);

	// 2Dオブジェクト
	uint32_t CreateObject2D(const std::string& textureName, const std::string& name,
		const std::string& groupName);

	// 空のオブジェクトを作成する
	uint32_t BuildEmptyobject(const std::string& name, const std::string& groupName);

	// 指定オブジェクトを破棄する
	void Destroy(uint32_t object);
	// 破棄対象フラグの立つオブジェクトを一括破棄する
	void DestroyAll();

	//--------- accessor -----------------------------------------------------

	// オブジェクトに結びつくデータT(必要に応じ可変長)を取得する
	template<class T, bool Flag = false>
	typename ObjectPool<T, Flag>::Storage* GetData(uint32_t object);

	// 型TのSystemを取得する
	template<class T>
	T* GetSystem() const;

	// プールを取得
	ObjectPoolManager* GetObjectPoolManager() const { return objectPoolManager_.get(); }

	// singleton
	static ObjectManager* GetInstance();
	static void Finalize();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Asset* asset_;
	ID3D12Device* device_;

	static ObjectManager* instance_;

	std::unique_ptr<ObjectPoolManager> objectPoolManager_;

	std::unique_ptr<SystemManager> systemManager_;
};

//============================================================================
//	ObjectManager templateMethods
//============================================================================

template<class T, bool Flag>
inline typename ObjectPool<T, Flag>::Storage* ObjectManager::GetData(uint32_t object) {

	return objectPoolManager_->GetData<T, Flag>(object);
}

template<class T>
inline T* ObjectManager::GetSystem() const {

	return systemManager_->GetSystem<T>();
}