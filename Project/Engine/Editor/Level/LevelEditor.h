#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>
#include <Engine/Editor/Level/SceneBuilder.h>

// c++
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
// front
class Asset;

//============================================================================
//	LevelEditor class
//============================================================================
class LevelEditor :
	IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	LevelEditor() :IGameEditor("LevelEditor") {};
	~LevelEditor() = default;

	void Init(const std::string& initSceneFile);

	void Update();

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const std::string jsonPath_ = "Level/ObjectData/";

	// 全てのobjectを管理
	std::unordered_map<Level::ObjectType, std::vector<std::unique_ptr<GameObject3D>>> objectsMap_;

	// builder
	std::unique_ptr<SceneBuilder> sceneBuilder_;

	Level::ObjectType currentSelectType_;   // 選択中のタイプ
	std::optional<int> currentSelectIndex_; // 選択インデックス

	// editor
	ImVec2 rightChildSize_;        // 右側
	ImVec2 buttonSize_;            // ボタンサイズ
	ImGuiTextFilter selectFilter_; // 検索フィルター

	//--------- functions ----------------------------------------------------

	// json
	void SaveObject(GameObject3D* object);

	// update
	void BuildObjects();
	void UpdateObjects();

	// editor
	void SelectObject();
	void EditObject();
};