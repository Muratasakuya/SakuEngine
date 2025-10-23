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
//	アプリ全体のライフサイクルを統括し、初期化/メインループ/シーン遷移/終了処理を管理する。
//============================================================================
class Framework {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	// COMやログなどの初期設定と各サブシステム構築を行う
	Framework();
	~Framework() = default;

	// メインループを回し、更新→描画→後処理を繰り返す
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

	// 1フレームの全体更新(入力/非同期アセット/シーンステート/エディタ連携)を行う
	void Update();
	// 実ゲーム更新(シーン/ビュー/当たり判定/オブジェクト/パーティクル/ポスプロ)を行う
	void UpdateScene();

	// 1フレームの描画フローを実行する
	void Draw();
	// レンダーパス切替とポストプロセス実行、デバッグビュー処理を行う
	void RenderPath(DxCommand* dxCommand);

	// シーン遷移要求やライン描画のリセットを行う
	void EndRequest();
	// 各モジュールの終了処理とCOM解放を行う
	void Finalize();

	//--------- LeakChecker ----------------------------------------------------

	// アプリ終了時にDXGIのLive Objectsを出力してリークを検出する
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