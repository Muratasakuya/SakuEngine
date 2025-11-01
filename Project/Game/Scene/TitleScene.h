#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Methods/IScene.h>

// controller
#include <Game/Objects/TitleScene/TitleController.h>

// scene
#include <Engine/Scene/Light/PunctualLight.h>
#include <Game/Camera/Title/TitleViewCamera.h>
#include <Game/Objects/SceneTransition/FadeTransition.h>

//============================================================================
//	TitleScene class
//	タイトルシーン
//============================================================================
class TitleScene :
	public IScene,
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	TitleScene() :IGameEditor("TitleScene") {}
	~TitleScene() = default;

	void Init() override;

	void Update() override;

	void ImGui() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// controller
	std::unique_ptr<TitleController> controller_;

	// scene
	std::unique_ptr<TitleViewCamera> titleViewCamera_;
	std::unique_ptr<PunctualLight> light_;
	std::unique_ptr<FadeTransition> fadeTransition_;
};