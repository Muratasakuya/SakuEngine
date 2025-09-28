#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Camera/3D/CameraPathData.h>

// front
class SceneView;
class BaseCamera;

//============================================================================
//	CameraPathController class
//============================================================================
class CameraPathController {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	explicit CameraPathController(SceneView* sceneView);
	~CameraPathController() = default;

	void Update(CameraPathData& data);

	void Evaluate(const CameraPathData& data, float t,
		Vector3& outTranslation, Quaternion& outRotation, float& outFovY) const;

	void ApplyToCamera(BaseCamera& camera, const Vector3& translation, const Quaternion& rotation, float fovY) const;

	//--------- accessor -----------------------------------------------------

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	SceneView* sceneView_;

	//--------- functions ----------------------------------------------------

};