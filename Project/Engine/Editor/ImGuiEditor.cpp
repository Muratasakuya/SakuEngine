#include "ImGuiEditor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Scene/SceneView.h>
#include <Lib/MathUtils/MathUtils.h>

// imgui表示
#include <Engine/Asset/AssetEditor.h>
#include <Engine/Asset/Asset.h>
#include <Engine/Editor/ImGuiObjectEditor.h>
#include <Engine/Editor/Manager/GameEditorManager.h>
#include <Engine/Utility/GameTimer.h>
#include <Engine/Utility/EnumAdapter.h>

// imgui
#include <ImGuizmo.h>

//============================================================================
//	ImGuiEditor classMethods
//============================================================================

void ImGuiEditor::Init(const D3D12_GPU_DESCRIPTOR_HANDLE& renderTextureGPUHandle,
	const D3D12_GPU_DESCRIPTOR_HANDLE& debugSceneRenderTextureGPUHandle) {

	renderTextureGPUHandle_ = renderTextureGPUHandle;
	debugSceneRenderTextureGPUHandle_ = debugSceneRenderTextureGPUHandle;

	// サイズの変更、移動不可
	windowFlag_ = ImGuiWindowFlags_None;

	// 初期状態は表示
	displayEnable_ = true;
	isPlayGame_ = true;
	editMode_ = false;
	isShowDemoWindow_ = false;

	gameViewSize_ = ImVec2(896.0f, 504.0f);
	debugViewSize_ = ImVec2(768.0f, 432.0f);
	sceneSidebarWidth_ = 66.0f;
	gizmoIconSize_ = 40.0f;
}

void ImGuiEditor::LoadGizmoIconTextures(Asset* asset) {

	// アイコン読み込み
	asset->LoadTexture("manipulaterTranslate", AssetLoadType::Synch);
	asset->LoadTexture("manipulaterRotate", AssetLoadType::Synch);
	asset->LoadTexture("manipulaterScale", AssetLoadType::Synch);

	// GPUHandleを設定
	gizmoIconGPUHandles_.emplace(GizmoTransformEnum::Translate,
		asset->GetGPUHandle("manipulaterTranslate"));
	gizmoIconGPUHandles_.emplace(GizmoTransformEnum::Rotate,
		asset->GetGPUHandle("manipulaterRotate"));
	gizmoIconGPUHandles_.emplace(GizmoTransformEnum::Scale,
		asset->GetGPUHandle("manipulaterScale"));

	// 選択物への自動カメラフォーカス
	// 各軸のカメラ固定回転
}

void ImGuiEditor::SetConsoleViewDescriptor(
	DescriptorHeapType type, const BaseDescriptor* descriptor) {

	descriptors_[type] = descriptor;
}

void ImGuiEditor::Display(SceneView* sceneView) {

	// imguiの表示切り替え
	// F11で行う
	if (displayEnable_) {
		if (Input::GetInstance()->TriggerKey(DIK_F10)) {

			displayEnable_ = false;
		}
	} else {

		if (Input::GetInstance()->TriggerKey(DIK_F10)) {

			displayEnable_ = true;
		}
	}

	if (!displayEnable_) {
		return;
	}

	if (isShowDemoWindow_) {

		ImGui::ShowDemoWindow();
	}

	MenuBar();

	// ドッキング設定
	ImGui::DockSpaceOverViewport
	(ImGui::GetMainViewport()->ID, ImGui::GetMainViewport(), ImGuiDockNodeFlags_None);
	// Guizmo
	ImGuizmo::BeginFrame();

	// layout操作
	EditLayout();

	// imguiの表示
	MainWindow(sceneView);

	Console();

	Hierarchy();

	Inspector();

	AssetEdit();
}

void ImGuiEditor::EditLayout() {

	if (!editMode_) {
		return;
	}

	ImGui::Begin("EditLayout");

	ImGui::DragFloat2("gameViewSize", &gameViewSize_.x, 1.0f);
	ImGui::DragFloat2("debugViewSize", &debugViewSize_.x, 1.0f);
	ImGui::DragFloat("sceneSidebarWidth", &sceneSidebarWidth_, 0.1f);
	ImGui::DragFloat("gizmoIconSize", &gizmoIconSize_, 0.1f);

	ImGui::End();
}

void ImGuiEditor::MenuBar() {

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Menu")) {

			ImGui::Checkbox("Play", &isPlayGame_);
			ImGui::Checkbox("EditLayout", &editMode_);
			ImGui::Checkbox("ShowDemo", &isShowDemoWindow_);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void ImGuiEditor::MainWindow(SceneView* sceneView) {

	ImGui::Begin("Game", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_NoFocusOnAppearing);

	GameMenuBar();

	ImGui::Image(ImTextureID(renderTextureGPUHandle_.ptr), gameViewSize_);

	SetInputArea(InputViewArea::Game, ImGui::GetItemRectMin(), ImGui::GetItemRectSize());
	ImGui::End();

	ImGui::Begin("Scene", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoMove);

	// メニューバー
	SceneMenuBar();
	// 設定
	SceneManipulate();
	ImGui::SameLine();

	ImGui::Image(ImTextureID(debugSceneRenderTextureGPUHandle_.ptr), debugViewSize_);

	SetInputArea(InputViewArea::Scene, ImGui::GetItemRectMin(), ImGui::GetItemRectSize());

	// ギズモ呼び出し
	GizmoContext gizmoContext{};
	gizmoContext.drawlist = ImGui::GetWindowDrawList();
	gizmoContext.rectMin = ImGui::GetItemRectMin();
	gizmoContext.rectSize = ImGui::GetItemRectSize();
	gizmoContext.orthographic = false;
	Math::ToColumnMajor(Matrix4x4::Transpose(sceneView->GetSceneCamera()->GetViewMatrix()), gizmoContext.view);
	Math::ToColumnMajor(Matrix4x4::Transpose(sceneView->GetSceneCamera()->GetProjectionMatrix()), gizmoContext.projection);
	ImGuiObjectEditor::GetInstance()->DrawManipulateGizmo(gizmoContext);

	ImGui::End();
}

void ImGuiEditor::SceneMenuBar() {

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Scene")) {

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void ImGuiEditor::SceneManipulate() {

	ImGui::BeginChild("SceneManipulate",
		ImVec2(sceneSidebarWidth_, debugViewSize_.y),
		ImGuiChildFlags_Border);

	// ギズモで使用するSRTをボタンで切り替える
	// Gizmo
	GizmoIcons icons{};
	icons.translate = static_cast<ImTextureID>(gizmoIconGPUHandles_[GizmoTransformEnum::Translate].ptr);
	icons.rotate = static_cast<ImTextureID>(gizmoIconGPUHandles_[GizmoTransformEnum::Rotate].ptr);
	icons.scale = static_cast<ImTextureID>(gizmoIconGPUHandles_[GizmoTransformEnum::Scale].ptr);
	icons.size = ImVec2(gizmoIconSize_, gizmoIconSize_);
	ImGuiObjectEditor::GetInstance()->GizmoToolbar(icons);

	ImGui::EndChild();
}

void ImGuiEditor::GameMenuBar() {

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Game")) {

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void ImGuiEditor::Console() {

	ImGui::Begin("Console");

	if (ImGui::BeginTabBar("ConsoleTabBar")) {

		if (ImGui::BeginTabItem("GameObject")) {

			GameTimer::ImGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("ResourceView")) {

			for (const auto& [type, ptr] : descriptors_) {

				ImGui::SeparatorText(EnumAdapter<DescriptorHeapType>::ToString(type));
				DrawDescriptorUsageBar(nullptr, ptr);
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
}

void ImGuiEditor::Hierarchy() {

	ImGui::Begin("Hierarchy", nullptr, windowFlag_);

	if (ImGui::BeginTabBar("Hierarchy")) {

		if (ImGui::BeginTabItem("GameObject")) {

			// scene内のobject選択
			ImGuiObjectEditor::GetInstance()->SelectObject();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Editor")) {

			// editorの選択
			GameEditorManager::GetInstance()->SelectEditor();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
}

void ImGuiEditor::Inspector() {

	ImGui::Begin("Inspector", nullptr, windowFlag_);

	// 選択されたものの操作
	if (ImGui::BeginTabBar("Inspector")) {

		if (ImGui::BeginTabItem("GameObject")) {

			// scene内のobject選択
			ImGuiObjectEditor::GetInstance()->EditObject();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Editor")) {

			// editorの選択
			GameEditorManager::GetInstance()->EditEditor();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

void ImGuiEditor::AssetEdit() {

	//AssetEditor::GetInstance()->EditLayout();

	ImGui::Begin("Asset", nullptr, windowFlag_);

	AssetEditor::GetInstance()->ImGui();

	ImGui::End();
}

void ImGuiEditor::SetInputArea(InputViewArea viewArea, const ImVec2& imMin, const ImVec2& imSize) {

	Input* input = Input::GetInstance();

	Vector2 min = Vector2(imMin.x, imMin.y);
	Vector2 size = Vector2(imSize.x, imSize.y);
	input->SetViewRect(viewArea, min, size);
}

void ImGuiEditor::DrawDescriptorUsageBar(const char* label, const BaseDescriptor* descriptor) {

	const uint32_t used = descriptor->GetUseDescriptorCount();
	const uint32_t max = descriptor->GetMaxDescriptorCount();
	const float useRate = (0 < max) ? std::clamp(
		static_cast<float>(used) / static_cast<float>(max), 0.0f, 1.0f) : 0.0f;
	char overlay[64]{};
	std::snprintf(overlay, sizeof(overlay), "%u / %u", used, max);

	// ラベル表示
	if (label && *label) {
		ImGui::TextUnformatted(label);
	}
	// 幅はウィンドウに合わせる
	ImGui::ProgressBar(useRate, ImVec2(320.0f, 24.0f), overlay);
}