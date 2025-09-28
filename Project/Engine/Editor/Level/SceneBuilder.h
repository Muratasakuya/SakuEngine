#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>
#include <Engine/Editor/Level/LevelStructures.h>

// c++
#include <string>
#include <optional>
#include <algorithm>
#include <functional>
// imgui
#include <imgui.h>

//============================================================================
//	SceneBuilder class
//============================================================================
class SceneBuilder {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SceneBuilder() = default;
	~SceneBuilder() = default;

	void Init(const std::string& jsonPath);

	void CreateObjectsMap(std::unordered_map<Level::ObjectType,
		std::vector<std::unique_ptr<GameObject3D>>>& objectsMap);

	void Reset();

	void ImGui();

	//--------- accessor -----------------------------------------------------

	void SetFile(const std::string& sceneFile);

	bool IsCreate() const { return fileName_.has_value(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	std::string jsonPath_;

	// 追加するときのファイル名
	std::optional<std::string> fileName_;

	// 追加する際に同じ名前なら削除させるかどうかを設定する
	bool idDeleteOnSameName_;

	// editor
	ImVec2 leftChildSize_; // 左側
	ImVec2 buttonSize_;    // ボタンサイズ

	//--------- functions ----------------------------------------------------

	// json
	Json LoadObjectFile(const std::string& identifier);
	void RecieveFile();

	// helper
	void BuildObjects(const Json& obj, std::unordered_map<Level::ObjectType,
		std::vector<std::unique_ptr<GameObject3D>>>& objectsMap);

	bool IsMeshObjectCreatable(const Json& obj) const;
	template<typename... Ts>
	bool CheckCollisionValid(const GameObject3D& object);

	Level::ObjectType GetObjectType(const std::string& objectTypeName);

	std::unique_ptr<GameObject3D> CreateObject(const Json& obj, Level::ObjectType objectType);
	std::unique_ptr<GameObject3D> CreateObjectPtr(Level::ObjectType objectType);

	void HandleDuplicateObject(std::vector<std::unique_ptr<GameObject3D>>& objects,
		const std::string& identifier);

	void ApplyTransform(GameObject3D& object, const Json& data);
	void ApplyMaterial(GameObject3D& object, const Json& data);
	void ApplyCollision(GameObject3D& object, const Json& data);
};

//============================================================================
//	SceneBuilder templateMethods
//============================================================================

template<typename... Ts>
inline bool SceneBuilder::CheckCollisionValid(const GameObject3D& object) {

	return (... || (dynamic_cast<const Ts*>(&object) != nullptr));
}