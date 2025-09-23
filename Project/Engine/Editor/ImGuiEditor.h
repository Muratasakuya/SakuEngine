#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/InputStructures.h>
#include <Engine/Core/Graphics/Descriptors/BaseDescriptor.h>

// directX
#include <d3d12.h>
// imgui
#include <imgui.h>
// c++
#include <unordered_map>
#include <optional>
// front
class SceneView;
class Asset;

//============================================================================
//	ImGuiEditor class
//============================================================================
class ImGuiEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ImGuiEditor() = default;
	~ImGuiEditor() = default;

	void Init(const D3D12_GPU_DESCRIPTOR_HANDLE& renderTextureGPUHandle,
		const D3D12_GPU_DESCRIPTOR_HANDLE& debugSceneRenderTextureGPUHandle);
	void LoadIconTextures(Asset* asset);

	void Display(SceneView* sceneView);

	//--------- accessor -----------------------------------------------------

	void SetConsoleViewDescriptor(DescriptorHeapType type,
		const BaseDescriptor* descriptor);

	bool IsPlayGame() const { return isPlayGame_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	enum class GizmoEnum {

		None,
		Translate,
		Rotate,
		Scale
	};

	//--------- variables ----------------------------------------------------

	// GPUHandles
	D3D12_GPU_DESCRIPTOR_HANDLE renderTextureGPUHandle_;
	D3D12_GPU_DESCRIPTOR_HANDLE debugSceneRenderTextureGPUHandle_;
	D3D12_GPU_DESCRIPTOR_HANDLE shadowTextureGPUHandle_;
	std::unordered_map<GizmoEnum, D3D12_GPU_DESCRIPTOR_HANDLE> gizmoIconGPUHandles_;
	D3D12_GPU_DESCRIPTOR_HANDLE cameraAutoFocusGPUHandle_;

	ImGuiWindowFlags windowFlag_;

	// imgui表示フラグ
	bool displayEnable_;
	// update処理を行うかのフラグ
	bool isPlayGame_;

	// parameter
	bool editMode_;
	ImVec2 gameViewSize_;
	ImVec2 debugViewSize_;
	float sceneSidebarWidth_;
	float gizmoIconSize_;

	// editor
	bool isShowDemoWindow_;
	bool isCameraAutoFocus_;
	std::optional<uint32_t> lastAutoFocusId_;

	// console
	std::unordered_map<DescriptorHeapType, const BaseDescriptor*> descriptors_;

	//--------- functions ----------------------------------------------------

	// layoutEditor
	void EditLayout();

	// menuBar
	void MenuBar();

	// renderTextureの描画
	void MainWindow(SceneView* sceneView);
	void SceneMenuBar();
	void SceneManipulate();
	void GameMenuBar();

	// console
	void Console();

	// hierarchy
	void Hierarchy(SceneView* sceneView);

	// inspector
	void Inspector();

	// asset
	void AssetEdit();

	// helper
	void SetInputArea(InputViewArea viewArea, const ImVec2& imMin, const ImVec2& imSize);
	void DrawDescriptorUsageBar(const char* label, const BaseDescriptor* descriptor);
};