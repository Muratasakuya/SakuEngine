#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Methods/IScene.h>

// scene
#include <Engine/Scene/Camera/BaseCamera.h>
#include <Engine/Scene/Light/PunctualLight.h>

//============================================================================
//	EffectScene class
//	エフェクト確認用シーン
//============================================================================
class EffectScene :
	public IScene {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	EffectScene() = default;
	~EffectScene() = default;

	void Init() override;

	void Update() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	std::unique_ptr<BaseCamera> camera3D_;
	std::unique_ptr<PunctualLight> light_;
};