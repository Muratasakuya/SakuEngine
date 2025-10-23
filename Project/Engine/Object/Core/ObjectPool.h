#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Config.h>

// c++
#include <cstdint>
#include <memory>
#include <vector>
#include <bitset>
#include <unordered_map>
#include <typeindex>
// imgui
#include <imgui.h>

// dataBitSize
constexpr const size_t kMaxDataTypes = 64;
using Archetype = std::bitset<kMaxDataTypes>;

//============================================================================
//	IObjectPool class
//	各プール共通のIFを定義し、削除/デバッグ操作を提供する。
//============================================================================
class IObjectPool {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IObjectPool() = default;
	virtual ~IObjectPool() = default;

	// 指定オブジェクトのデータを削除する
	virtual void Remove(uint32_t object) = 0;

	// プール状態をデバッグ表示する
	virtual void Debug(const char* label) = 0;
};

//============================================================================
//	ObjectPool class
//	汎用データプール。追加/削除/取得とメモリ再利用を管理する。
//============================================================================
template<class T, bool kMultiple = false>
class ObjectPool :
	public IObjectPool {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	explicit ObjectPool();
	~ObjectPool() = default;

	//--------- variables ----------------------------------------------------

	// アクセス番地
	// object -> index
	std::unordered_map<uint32_t, size_t> objectToIndex_;
	// index -> object
	std::vector<uint32_t> indexToObject_;
	std::vector<size_t> freeList_;

	// data
	// kMultiple = true: std::vector<T>
	// kMultiple = false: T
	using Storage = std::conditional_t<kMultiple, std::vector<T>, T>;
	std::vector<Storage> data_;

	//--------- functions ----------------------------------------------------

	// imguiでプールの容量/連続性/配置をデバッグ表示する
	void Debug(const char* label) override;

	// オブジェクトにデータを追加/上書きしインデックスを更新する
	template<class... Args>
	void Add(uint32_t object, Args&&... args);
	// 指定オブジェクトのデータを削除する
	void Remove(uint32_t object) override;
	// 指定オブジェクトのデータを取得する(無ければnullptr)
	Storage* Get(uint32_t object);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- functions ----------------------------------------------------

	// 内部処理: 実削除と空き番地管理を行う
	void RemoveImpl(uint32_t object);
};

//============================================================================
//	ObjectPool templateMethods
//============================================================================

template<class T, bool kMultiple>
template<class ...Args>
inline void ObjectPool<T, kMultiple>::Add(uint32_t object, Args && ...args) {

	// すでに持っていれば上書き
	auto it = objectToIndex_.find(object);
	if (it != objectToIndex_.end()) {

		data_[it->second] = Storage{ std::forward<Args>(args)... };
		return;
	}

	size_t index;
	if (!freeList_.empty()) {

		// 空いているindexを取得し再利用する
		index = freeList_.back();
		freeList_.pop_back();
		data_[index] = Storage{ std::forward<Args>(args)... };
		indexToObject_[index] = object;
	} else {

		// capacityを超えたら
		ASSERT(data_.size() < data_.capacity(), "ObjectPool capacity exceeded");

		index = data_.size();
		data_.emplace_back(Storage{ std::forward<Args>(args)... });
		indexToObject_.push_back(object);
	}
	objectToIndex_[object] = index;
}

template<class T, bool kMultiple>
inline ObjectPool<T, kMultiple>::ObjectPool() {

	// 最初に最大数を確保、これ以降は禁止
	data_.reserve(Config::kMaxInstanceNum);
}

template<class T, bool kMultiple>
inline void ObjectPool<T, kMultiple>::Debug(const char* label) {

	ImGui::PushItemWidth(224.0f);

	if (!ImGui::CollapsingHeader(label)) {
		ImGui::PopItemWidth();
		return;
	}

	ImGui::Text("size      = %zu", data_.size());
	ImGui::Text("capacity  = %zu", data_.capacity());
	ImGui::Text("element   = %zu bytes", sizeof(Storage));

	// 連続性チェック
	bool contiguous = true;
	for (size_t i = 1; i < data_.size(); ++i) {

		uintptr_t prev = reinterpret_cast<uintptr_t>(&data_[i - 1]);
		uintptr_t curr = reinterpret_cast<uintptr_t>(&data_[i]);
		// 差分をStorageと比較
		if (curr - prev != sizeof(Storage)) {
			contiguous = false;
			break;
		}
	}
	// 連続していれば緑、していなければ赤
	ImGui::TextColored(contiguous ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f)
		: ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
		"contiguous : %s", contiguous ? "YES" : "NO");

	// メモリアドレス一覧
	const std::string labelStr = std::string("MemoryArray##") + label;
	if (ImGui::TreeNode(labelStr.c_str())) {

		ImGui::Text("Index   Address");
		ImGui::Separator();

		uintptr_t prevAddr = 0;
		for (size_t i = 0; i < data_.size(); ++i) {

			uintptr_t addr = reinterpret_cast<uintptr_t>(&data_[i]);
			if (i == 0) {

				ImGui::Text("[%4zu]  0x%016llx      -", i, static_cast<unsigned long long>(addr));
			} else {

				ImGui::Text("[%4zu]  0x%016llx   %+6lld", i,
					static_cast<unsigned long long>(addr),
					static_cast<long long>(addr - prevAddr));
			}

			prevAddr = addr;
		}
		ImGui::TreePop();
	}

	ImGui::PopItemWidth();
}

template<class T, bool kMultiple>
inline void ObjectPool<T, kMultiple>::Remove(uint32_t object) {

	RemoveImpl(object);
}

template<class T, bool kMultiple>
inline ObjectPool<T, kMultiple>::Storage* ObjectPool<T, kMultiple>::Get(uint32_t object) {

	auto it = objectToIndex_.find(object);
	// objectが存在していれば値を返す
	if (it != objectToIndex_.end()) {

		return  &data_[it->second];
	}
	// 存在していなければnullptrを返す
	return nullptr;
}

template<class T, bool kMultiple>
inline void ObjectPool<T, kMultiple>::RemoveImpl(uint32_t object) {

	// 存在しないobjectの場合処理しない
	auto it = objectToIndex_.find(object);
	if (it == objectToIndex_.end()) {
		return;
	}

	size_t index = it->second;
	data_[index] = Storage{};
	indexToObject_[index] = 0xffffffff;

	// 空き番地を記録
	objectToIndex_.erase(it);
	freeList_.push_back(index);
}