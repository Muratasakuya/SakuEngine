#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Editor/ImGuiEditor.h>

// core
#include <Engine/Core/Window/WinApp.h>
#include <Engine/Core/Graphics/GraphicsPlatform.h>
#include <Engine/Core/Graphics/RenderEngine.h>
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>

// scene
#include <Engine/Scene/Manager/SceneManager.h>
#include <Engine/Scene/SceneView.h>

//============================================================================
//	Framework class
//============================================================================
class Framework {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Framework();
	~Framework() = default;

	void Run();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	std::unique_ptr<WinApp> winApp_;
	bool fullscreenEnable_;

	std::unique_ptr<GraphicsPlatform> graphicsPlatform_;
	std::unique_ptr<RenderEngine> renderEngine_;

	std::unique_ptr<Asset> asset_;

	std::unique_ptr<ImGuiEditor> imguiEditor_;

	std::unique_ptr<SceneManager> sceneManager_;

	std::unique_ptr<SceneView> sceneView_;

	//--------- functions ----------------------------------------------------

	// update
	void Update();
	void UpdateScene();

	// draw
	void Draw();
	void RenderPath(DxCommand* dxCommand);

	void EndRequest();
	void Finalize();

	//--------- LeakChecker ----------------------------------------------------

	struct LeakChecker {

		~LeakChecker() {

			ComPtr<IDXGIDebug1> debug;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(debug.GetAddressOf())))) {

				debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
				debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
				debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
			}
		}
	};
	LeakChecker leakChecker_;
};