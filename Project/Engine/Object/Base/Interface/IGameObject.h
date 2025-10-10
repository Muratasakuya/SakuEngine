#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/PostProcessBit.h>
#include <Engine/Utility/Enum/ObjectUpdateMode.h>

// data
#include <Engine/Object/Data/Transform.h>
#include <Engine/Object/Data/Material.h>
#include <Engine/Object/Data/ObjectTag.h>

// c++
#include <string>
#include <optional>
#include <cstdint>
// front
class ObjectManager;

//============================================================================
//	IGameObject class
//============================================================================
class IGameObject {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IGameObject();
	virtual ~IGameObject();

	virtual void DerivedInit() = 0;

	virtual void ImGui() = 0;
	virtual void DerivedImGui() = 0;

	//--------- accessor -----------------------------------------------------

	/*---------- setter ----------*/

	void SetIdentifier(const std::string& identifier) { identifier_ = identifier; }
	void SetDestroyOnLoad(bool enable) { tag_->destroyOnLoad = enable; }

	/*---------- getter ----------*/

	const ObjectTag& GetTag() const { return *tag_; }
	const std::string& GetIdentifier() const { return identifier_; }
	uint32_t GetObjectID() const { return objectId_; }
	ObjectUpdateMode GetUpdateMode() const { return updateMode_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ObjectManager* objectManager_;

	// 自身のobjectId
	uint32_t objectId_;
	// tag
	ObjectTag* tag_;

	// 更新方法
	ObjectUpdateMode updateMode_ = ObjectUpdateMode::None;

	// 序列関係なしの名前
	std::string identifier_;

	// imgui
	const float itemWidth_ = 224.0f;
};