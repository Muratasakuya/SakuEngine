#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/System/Base/ISystem.h>
#include <Engine/Object/Data/ObjectTag.h>

// c++
#include <string>
#include <unordered_map>

//============================================================================
//	TagSystem class
//============================================================================
class TagSystem :
	public ISystem {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	TagSystem() = default;
	~TagSystem() = default;

	Archetype Signature() const override;

	void Update(ObjectPoolManager& ObjectPoolManager) override;

	std::string CheckName(const std::string& name);

	//--------- accessor -----------------------------------------------------

	const std::unordered_map<std::string, std::vector<uint32_t>>& Groups() const { return groups_; }
	const std::unordered_map<uint32_t, const ObjectTag*>& Tags() const { return idToTag_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// dataのキャッシュデータ
	std::unordered_map<std::string, std::vector<uint32_t>> groups_;
	std::unordered_map<uint32_t, const ObjectTag*> idToTag_;

	// object数のキャッシュ
	size_t prevobjectCount_;

	// 名前の累計カウント
	std::unordered_map<std::string, int> nameCounts_;

	//--------- functions ----------------------------------------------------

	std::string SplitBaseNameAndNumber(const std::string& name, int& number);
};