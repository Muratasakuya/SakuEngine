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

		bool isSynch = false; // 同期するか
		bool isLoop = false;  // ループ再生
		float time;           // 現在の時間

		// 現在のキーフレーム
		int selectedKeyIndex = 0;
	};

	// 同期オブジェクト
	struct ActionSynchBind {

		std::string objectName;
		std::string spanName;

		// 同期の仕方
		// true:  カメラ->状態
		// false: 状態->カメラ
		bool driveStateFromCamera = true;
	};
public:
	//========================================================================
	//	public Methods
	//========================================================================

	explicit CameraPathController(SceneView* sceneView);
	~CameraPathController() = default;

	void Update(const PlaybackState& state, CameraPathData& data);

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