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
//	カメラキーパス補間エディターパネル
//============================================================================
class Camera3DEditorPanel {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Camera3DEditorPanel() = default;
	~Camera3DEditorPanel() = default;

	// 全てのエディターUIの呼び出し
	void Edit(std::unordered_map<std::string, CameraPathData>& params,
		std::vector<std::string>& names, std::string& selectedParamKey, int& selectedKeyIndex,
		JsonSaveState& paramSaveState, char lastLoaded[128],
		CameraPathController::PlaybackState& playbackCamera);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- functions ----------------------------------------------------

	// アニメーションキー追加
	void AddAnimationKey(std::vector<std::string>& names,
		std::unordered_map<std::string, CameraPathData>& params,
		std::string& selectedName);
	// 値調整データの選択
	void SelectCameraParam(std::unordered_map<std::string, CameraPathData>& params,
		std::string& selectedParamKey);
	// 調整データの保存、読み込み
	void SaveAndLoad(CameraPathData& param, JsonSaveState& paramSaveState, char lastLoaded[128]);

	// 値調整
	void EditCameraParam(CameraPathData& param, std::string& selectedParamKey, int& selectedKeyIndex,
		JsonSaveState& paramSaveState, char lastLoaded[128],
		CameraPathController::PlaybackState& playbackCamera);

	// エディターのタブ値調整
	// 再生
	void EditPlayback(CameraPathData& param, CameraPathController::PlaybackState& playbackCamera);
	// 補間の仕方設定
	void EditLerp(CameraPathData& param);
	// キーフレームの追加、削除、入れ替え
	void EditKeyframe(CameraPathData& param, int& selectedKeyIndex);
	// 追従先の設定
	void SelectTarget(CameraPathData& param);
};