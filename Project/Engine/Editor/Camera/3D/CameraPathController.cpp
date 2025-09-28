#include "CameraPathController.h"

//============================================================================
//	CameraPathController classMethods
//============================================================================

CameraPathController::CameraPathController(SceneView* sceneView) {

	sceneView_ = nullptr;
	sceneView_ = sceneView;
}
//
//void CameraPathController::Update(CameraPathData& data) {
//
//
//}
//
//void CameraPathController::Evaluate(const CameraPathData& data, float t, Vector3& outTranslation,
//	Quaternion& outRotation, float& outFovY) const {
//
//
//}
//
//void CameraPathController::ApplyToCamera(BaseCamera& camera, const Vector3& translation,
//	const Quaternion& rotation, float fovY) const {
//
//
//}