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
//============================================================================
class ObjectManager {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ObjectManager() = default;
	~ObjectManager() = default;

	void Init(ID3D12Device* device, Asset* asset, DxCommand* dxCommand);

	void UpdateData();
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

	// object削除
	void Destroy(uint32_t object);
	void DestroyAll();

	//--------- accessor -----------------------------------------------------

	template<class T, bool Flag = false>
	typename ObjectPool<T, Flag>::Storage* GetData(uint32_t object);

	template<class T>
	T* GetSystem() const;

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

	//--------- functions ----------------------------------------------------

	uint32_t BuildEmptyobject(const std::string& name, const std::string& groupName);
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