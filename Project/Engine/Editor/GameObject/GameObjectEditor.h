#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>
#include <Engine/Object/Base/GameObject3D.h>

// front
class ObjectManager;
class Asset;

//============================================================================
//	GameObjectEditor class
//============================================================================
class GameObjectEditor :
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameObjectEditor() :IGameEditor("GameObjectEditor") {}
	~GameObjectEditor() = default;

	void Init(Asset* asset);

	void Update();

	void ImGui() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	enum class objectClassType {

		None, // 通常のクラス
	};

	struct objectHandle {

		objectClassType classType;  // どのクラスのvectorか
		size_t innerIndex;          // vector内での位置
	};

	struct InputTextValue {

		char nameBuffer[128]; // imgui入力用
		std::string name;     // 入力した文字を取得する用

		void Reset();
	};

	//--------- variables ----------------------------------------------------

	ObjectManager* ObjectManager_;
	Asset* asset_;

	// 3Dobjectのデータ
	std::unordered_map<objectClassType, std::vector<std::unique_ptr<GameObject3D>>> entities_;

	// object追加入力処理
	InputTextValue addNameInputText_;             // 名前
	std::optional<std::string> addModelName_;     // 追加するモデルの名前
	std::optional<std::string> addAnimationName_; // 追加するアニメーションの名前
	objectClassType addClassType_;                // 追加するクラスのタイプ
	// 選択されたobjectの名前
	std::optional<std::string> selectobjectName_;

	// 追加されたobjectの名前list
	std::vector<std::string> objectNames_;
	std::vector<objectHandle> objectHandles_;
	std::optional<int> currentSelectIndex_;

	// editor
	ImVec2 addButtonSize_;  // 追加ボタンサイズ
	ImVec2 leftChildSize_;  // 左側
	ImVec2 rightChildSize_; // 右側
	ImVec2 dropSize_;       // ドロップ処理受け取りサイズ

	//--------- functions ----------------------------------------------------

	void EditLayout();

	// update
	void UpdateEntities();

	// 追加処理
	void Addobject();
	// 選択処理
	void Selectobject();
	// 操作処理
	void Editobject();
	// 削除処理
	void Removeobject();

	// helper
	void DropFile(const std::string& label, std::optional<std::string>& recieveName);

	void SelectobjectClassType(objectClassType& classType);
};