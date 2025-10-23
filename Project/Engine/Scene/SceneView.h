#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>

// buffer
#include <Engine/Core/Graphics/GPUObject/SceneConstBuffer.h>

// camera
#include <Engine/Scene/Camera/BaseCamera.h>
#include <Engine/Scene/Camera/DebugCamera.h>
#include <Engine/Scene/Camera/Camera2D.h>

// light
#include <Engine/Scene/Light/PunctualLight.h>

// dither
#include <Engine/Core/Graphics/GPUObject/DitherStructures.h>

// c++
#include <memory>
#include <optional>

//============================================================================
//	SceneView class
//	シーンの環境を更新、提供するクラス
//============================================================================
class SceneView :
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SceneView() : IGameEditor("SceneView") {};
	~SceneView() = default;

	void Init();

	void Update();

	void ImGui() override;
	//--------- accessor -----------------------------------------------------

	// camera
	void SetGameCamera(BaseCamera* gameCamera);
	void AddSceneCamera(const std::string& name, BaseCamera* sceneCamera);
	// light
	void SetLight(PunctualLight* gameLight);

	// camera
	BaseCamera* GetCamera() const { return activeGameCamera3D_.value(); }
	BaseCamera* GetSceneCamera() const { return activeSceneCamera_.value(); }
	Camera2D* GetCamera2D() const { return camera2D_.get(); }
	// light
	PunctualLight* GetLight() const { return punctualLight_.value(); }
	// dither
	const DitherForGPU& GetDither() const { return dither_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// camera
	// 3Dシーン
	std::optional<BaseCamera*> activeGameCamera3D_; // ゲームで使用されているカメラ
	std::optional<BaseCamera*> activeSceneCamera_;  // シーン視点のカメラ

	// シーン視点のカメラの配列、この中からシーン視点のカメラを選択する
	std::unordered_map<std::string, std::vector<BaseCamera*>> sceneCameras_;

	// デバッグカメラ
	std::unique_ptr<DebugCamera> debugCamera_;
	// 2Dシーン
	std::unique_ptr<Camera2D> camera2D_;

	// light
	std::optional<PunctualLight*> punctualLight_;

	// dither
	DitherForGPU dither_;

	// editor
	int activeSceneCameraIndex_;

	//--------- functions ----------------------------------------------------

	// init
	void InitCamera();

	// update
	void UpdateCamera();
	void UpdateLight();

	void EditCamera();
	void EditLight();

	// debug
	void DisplayPointLight();
	void DisplaySpotLight();
};