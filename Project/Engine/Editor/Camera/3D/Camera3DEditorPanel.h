#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Camera/3D/CameraPathData.h>
#include <Engine/Editor/Camera/3D/CameraPathGizmoSynch.h>
#include <Engine/Editor/Camera/3D/CameraPathRenderer.h>
#include <Engine/Editor/Camera/3D/CameraPathController.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

//============================================================================
//	Camera3DEditorPanel class
//============================================================================
class Camera3DEditorPanel {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Camera3DEditorPanel() = default;
	~Camera3DEditorPanel() = default;

	void Edit(std::unordered_map<std::string, CameraPathData>& params,
		std::unordered_map<std::string, const SkinnedAnimation*>& skinnedAnimations,
		std::string& selectedSkinnedKey, std::string& selectedAnimName,
		std::string& selectedParamKey, int& selectedKeyIndex,
		JsonSaveState& paramSaveState, char lastLoaded[128],
		CameraPathController::PlaybackState& playbackCamera);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- functions ----------------------------------------------------

	void SelectAnimationSubject(std::unordered_map<std::string, const SkinnedAnimation*>& skinnedAnimations,
		std::string& selectedSkinnedKey, std::string& selectedAnimName);
	void AddCameraParam(std::unordered_map<std::string, CameraPathData>& params,
		std::string& selectedAnimName);
	void SelectCameraParam(std::unordered_map<std::string, CameraPathData>& params,
		std::string& selectedParamKey);
	void EditCameraParam(CameraPathData& param, std::unordered_map<std::string, const SkinnedAnimation*>& skinnedAnimations,
		std::string& selectedSkinnedKey, std::string& selectedParamKey,
		int& selectedKeyIndex, JsonSaveState& paramSaveState, char lastLoaded[128],
		CameraPathController::PlaybackState& playbackCamera);

	// edit
	void SaveAndLoad(CameraPathData& param, JsonSaveState& paramSaveState, char lastLoaded[128]);
	void EditPlayback(CameraPathData& param, CameraPathController::PlaybackState& playbackCamera);
	void EditLerp(CameraPathData& param, std::unordered_map<std::string, const SkinnedAnimation*>& skinnedAnimations,
		std::string& selectedSkinnedKey, std::string& selectedParamKey);
	void EditKeyframe(CameraPathData& param, int& selectedKeyIndex);
	void SelectTarget(CameraPathData& param);
};