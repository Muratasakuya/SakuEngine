#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <cstdint>
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <unordered_map>
#include <algorithm>
#include <functional>
// imgui
#include <imgui.h>
#include <ImGuizmo.h>
// front
class TagSystem;
class ObjectManager;
class IGameObject;

//============================================================================
//	ImGuiObjectEditor structure
//============================================================================

// ギズモ描画情報
struct GizmoContext {

	ImDrawList* drawlist; // ウィンドウのDrawList
	ImVec2 rectMin;       // ImGui::Imageの左上
	ImVec2 rectSize;      // サイズ
	float view[16];
	float projection[16];
	bool orthographic;
};
// ギズモアイコン情報
struct GizmoIcons {

	ImTextureID none;
	ImTextureID translate;
	ImTextureID rotate;
	ImTextureID scale;
	ImVec2 size;
};

//============================================================================
//	ImGuiObjectEditor class
//============================================================================
class ImGuiObjectEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	// 初期化
	void Init();

	// objectの選択
	void SelectObject();
	// 選択したobjectの操作
	void EditObject();

	// 選択全解除
	void Reset();
	void Registerobject(uint32_t id, IGameObject* object);

	// ギズモ呼び出し
	void DrawManipulateGizmo(const GizmoContext& context);
	void GizmoToolbar(const GizmoIcons& icons);
	bool IsUsingGuizmo() const { return isUsingGuizmo_; }

	//--------- accessor -----------------------------------------------------

	// 外部からフォーカスするIDを設定
	void SelectById(uint32_t id);
	std::optional<uint32_t> GetSelected3D() const { return selected3D_; }
	bool IsPickActive() const { return isPickActive_; }

	// singleton
	static ImGuiObjectEditor* GetInstance();
	static void Finalize();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	struct EditImGui {

		// imguiで選択されたidの保持
		std::optional<uint32_t> selectedId_ = std::nullopt;
		std::unordered_map<uint32_t, std::function<void()>> imguiFunc_;
	};

	//--------- variables ----------------------------------------------------

	static ImGuiObjectEditor* instance_;

	TagSystem* tagSystem_;
	ObjectManager* objectManager_;

	std::unordered_map<std::string, std::vector<uint32_t>> groups_;

	std::unordered_map<uint32_t, std::optional<IGameObject*>> objectsMap_;

	std::optional<uint32_t> selected3D_;
	std::optional<uint32_t> displayed3D_;
	int selectedMaterialIndex_ = 0;

	std::optional<uint32_t> selected2D_;
	int selectedSpriteIndex_ = 0;

	// Guizmo
	ImGuizmo::OPERATION currentOption_ = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE currentMode_ = ImGuizmo::WORLD;
	bool isUsingGuizmo_ = false;
	bool isPickActive_ = true;
	bool isAutoSelect_ = true;

	const float itemWidth_ = 192.0f;

	//--------- functions ----------------------------------------------------

	ImGuiObjectEditor() = default;
	~ImGuiObjectEditor() = default;
	ImGuiObjectEditor(const ImGuiObjectEditor&) = delete;
	ImGuiObjectEditor& operator=(const ImGuiObjectEditor&) = delete;

	// 3Dか2Dかの判定
	bool Is3D(uint32_t object) const;
	bool Is2D(uint32_t object) const;
	// 選択可能なobjectの描画
	void DrawSelectable(uint32_t object, const std::string& name);
	// 表示に使うID
	std::optional<uint32_t> CurrentInfo3D() const;

	//----------- group ------------------------------------------------------

	// groupの作成
	void CreateGroup();
	// group化されたobjectの選択
	void SelectGroupedObject();

	//--------- Objects -----------------------------------------------------

	// Object詳細、操作
	void EditObjects();
	void ObjectsInformation();
	void ObjectsTransform();
	void ObjectsMaterial();

	void EditSkybox();

	//--------- object2D -----------------------------------------------------

	// Object2D詳細、操作
	void EditObject2D();
	void Object2DInformation();
	void Object2DSprite();
	void Object2DTransform();
	void Object2DMaterial();
};