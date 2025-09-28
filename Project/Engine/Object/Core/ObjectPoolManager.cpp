#include "ObjectPoolManager.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Helper/Algorithm.h>

// data
#include <Engine/Object/Data/Transform.h>
#include <Engine/Object/Data/Material.h>
#include <Engine/Object/Data/Animation.h>
#include <Engine/Object/Data/ObjectTag.h>
#include <Engine/Object/Data/Sprite.h>

//============================================================================
//	ObjectPoolManager classMethods
//============================================================================

uint32_t ObjectPoolManager::Create() {

	return alive_.empty() ? next_++ : PopAlive();
}

void ObjectPoolManager::Destroy(uint32_t object) {

	Archetype arch = objectToArch_[object];

	// bitが立っている全タイプのpoolからRemove
	for (size_t typeId = 0; typeId < kMaxDataTypes; ++typeId) {
		if (!arch.test(typeId) || typeId >= pools_.size()) {
			continue;
		}
		if (pools_[typeId]) {

			pools_[typeId]->Remove(object);
		}
	}

	// archetypeテーブルから除去
	if (auto it = objectToArch_.find(object); it != objectToArch_.end()) {

		archToEntities_[it->second].erase(
			std::remove(archToEntities_[it->second].begin(),
				archToEntities_[it->second].end(), object),
			archToEntities_[it->second].end());
		objectToArch_.erase(it);
	}
	// 再利用キューへ
	alive_.push_back(object);
}

std::vector<uint32_t> ObjectPoolManager::View(const Archetype& mask) const {

	std::vector<uint32_t> result;
	for (auto& [arch, list] : archToEntities_) {
		if ((arch & mask) == mask) {

			result.insert(result.end(), list.begin(), list.end());
		}
	}
	return result;
}

void ObjectPoolManager::ImGui() {

	// 登録済みプールを列挙
	for (size_t id = 0; id < pools_.size(); ++id) {
		if (!pools_[id]) continue;  // 未生成

		// 型名を取るなら RTTI 等を使う、ここではインデックスで表示
		std::string label = std::string(typeid(*pools_[id]).name()) + " [" + std::to_string(id) + "]";
		label = Algorithm::RemoveSubstring(label, "class");
		label = Algorithm::RemoveSubstring(label, "struct");
		label = Algorithm::RemoveSubstring(label, "ObjectPool");
		pools_[id]->Debug(label.c_str());
	}
}

template<class T>
void ObjectPoolManager::SetBit(uint32_t object, bool enable) {

	size_t id = GetTypeID<T>();

	Archetype& arch = objectToArch_[object];
	Archetype  old = arch;

	arch.set(id, enable);

	// 旧リスト削除
	auto& oldList = archToEntities_[old];
	oldList.erase(std::remove(oldList.begin(), oldList.end(), object), oldList.end());

	// 新リスト追加
	archToEntities_[arch].push_back(object);
}

// 各dataを明示的にインスタンス化
template void ObjectPoolManager::SetBit<class Transform3D>(uint32_t, bool);
template void ObjectPoolManager::SetBit<class Transform2D>(uint32_t, bool);
template void ObjectPoolManager::SetBit<class Material>(uint32_t, bool);
template void ObjectPoolManager::SetBit<class SpriteMaterial>(uint32_t, bool);
template void ObjectPoolManager::SetBit<class SkinnedAnimation>(uint32_t, bool);
template void ObjectPoolManager::SetBit<class Sprite>(uint32_t, bool);
template void ObjectPoolManager::SetBit<class Skybox>(uint32_t, bool);
template void ObjectPoolManager::SetBit<struct ObjectTag>(uint32_t, bool);

uint32_t ObjectPoolManager::PopAlive() {

	uint32_t object = alive_.back(); alive_.pop_back(); return object;
}