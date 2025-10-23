#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectPool.h>
#include <Engine/Editor/Base/IGameEditor.h>

//============================================================================
//	ObjectPoolManager class
//	各型のObjectPoolとアーキタイプを統括し、追加/削除/参照とデバッグを提供する。
//============================================================================
class ObjectPoolManager :
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ObjectPoolManager() :IGameEditor("ObjectPoolManager") {};
	~ObjectPoolManager() = default;

	// 新規オブジェクトIDを発行する(再利用キュー優先)
	uint32_t Create();
	// 指定オブジェクトを破棄し、関連データとアーキタイプを更新する
	void Destroy(uint32_t object);

	// 型Tのプールを取得する(未生成なら生成して返す)
	template<class T, bool Flag = false>
	ObjectPool<T, Flag>& GetPool();

	// オブジェクトに型Tのデータを追加しビットを立てる
	template<class T, bool Flag = false, class... Args>
	typename ObjectPool<T, Flag>::Storage* AddData(uint32_t object, Args&&... args);
	// オブジェクトから型Tのデータを削除しビットを下ろす
	template<class T, bool Flag = false>
	void RemoveData(uint32_t object);

	// マスクを満たすアーキタイプのオブジェクト一覧を返す
	std::vector<uint32_t> View(const Archetype& mask) const;

	// 登録済みプールをimguiでデバッグ表示する
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// 型Tの内部IDを取得する
	template<class T>
	static size_t GetTypeID();

	// オブジェクトに結びつく型Tのデータを取得する
	template<class T, bool Flag = false>
	typename ObjectPool<T, Flag>::Storage* GetData(uint32_t object);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	std::unordered_map<uint32_t, Archetype> objectToArch_;
	std::unordered_map<Archetype, std::vector<uint32_t>> archToEntities_;

	std::vector<std::shared_ptr<IObjectPool>> pools_;

	uint32_t next_ = 1;
	std::vector<uint32_t> alive_;

	inline static std::atomic_size_t typeCounter_ = 0;

	//--------- structure ----------------------------------------------------

	// 型TごとにユニークなIDを保持するホルダー
	template<class T>
	struct TypeIDHolder {

		inline static const size_t value = ++typeCounter_;
	};

	//--------- functions ----------------------------------------------------

	// 内部: アーキタイプのビット更新とエンティティリストの再構成を行う
	template<class T>
	void SetBit(uint32_t object, bool enable);

	// 内部: 再利用キューからIDを取り出す
	uint32_t PopAlive();
};

template<class T, bool Flag>
inline ObjectPool<T, Flag>& ObjectPoolManager::GetPool() {

	size_t id = GetTypeID<T>();
	if (id >= pools_.size()) {
		pools_.resize(id + 1);
	}
	if (!pools_[id]) {
		pools_[id] = std::make_shared<ObjectPool<T, Flag>>();
	}
	return *static_cast<ObjectPool<T, Flag>*>(pools_[id].get());
}

template<class T, bool Flag, class... Args>
inline auto ObjectPoolManager::AddData(uint32_t object, Args&&... args)
-> typename ObjectPool<T, Flag>::Storage* {

	auto& pool = GetPool<T, Flag>();
	pool.Add(object, std::forward<Args>(args)...);
	SetBit<T>(object, true);
	return GetData<T, Flag>(object);
}

template<class T, bool Flag>
inline void ObjectPoolManager::RemoveData(uint32_t object) {

	GetPool<T, Flag>().Remove(object);
	SetBit<T>(object, false);
}

template<class T, bool Flag>
inline typename ObjectPool<T, Flag>::Storage* ObjectPoolManager::GetData(uint32_t object) {

	return GetPool<T, Flag>().Get(object);
}

template<class T>
inline size_t ObjectPoolManager::GetTypeID() {

	return TypeIDHolder<T>::value;
}