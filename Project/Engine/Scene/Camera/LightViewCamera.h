#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Camera/BaseCamera.h>

//============================================================================
//	LightViewCamera class
//	光源視点のカメラ
//============================================================================
class LightViewCamera :
	public BaseCamera {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	LightViewCamera() = default;
	~LightViewCamera() = default;

	void Init() override;

	void Update() override;

	void ImGui() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Vector3 translation_; //* ライトの位置
	Vector3 target_;      //* 注視点
	Vector3 upDirection_; //* 上方向

	float orthoSize_;

	float nearPlane_;
	float farPlane_;
};