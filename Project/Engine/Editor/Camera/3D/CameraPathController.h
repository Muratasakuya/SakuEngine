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
//	カメラのキーパスの補間を行う
//============================================================================
class CameraPathController {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 再生状態
	enum class PreviewMode {

		Keyframe, // 操作しているキーフレームで表示
		Manual,   // 手動で動かす
		Play      // 再生
	};
	struct PlaybackState {

		bool isActive = false;
		PreviewMode mode = PreviewMode::Manual;

		bool isLoop = false;  // ループ再生
		float time;           // 現在の時間

		// 現在のキーフレーム
		int selectedKeyIndex = 0;
	};
public:
	//========================================================================
	//	public Methods
	//========================================================================

	explicit CameraPathController(SceneView* sceneView);
	~CameraPathController() = default;

	// カメラ補間データの更新
	void Update(const PlaybackState& state, CameraPathData& data);

	// カメラ補間値の取得
	void Evaluate(const CameraPathData& data, float t,
		Vector3& outTranslation, Quaternion& outRotation, float& outFovY) const;
	void EvaluateAtKey(const CameraPathData& data, int keyIndex,
		Vector3& outTranslation, Quaternion& outRotation, float& outFovY) const;

	// カメラ適応
	void ApplyToCamera(BaseCamera& camera, const Vector3& translation,
		const Quaternion& rotation, float fovY, bool isUseGame) const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	SceneView* sceneView_;

	//--------- functions ----------------------------------------------------

};