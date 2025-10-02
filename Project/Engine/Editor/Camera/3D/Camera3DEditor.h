#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Camera/3D/Camera3DEditorPanel.h>
#include <Engine/Editor/Base/IGameEditor.h>

// front
class SceneView;

//============================================================================
//	Camera3DEditor class
//============================================================================
class Camera3DEditor :
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	void Init(SceneView* sceneView);

	void Update();

	// 調整を行えるアクションの追加
	void AddActionObject(const std::string& name);
	// アクション同期の設定
	void SetActionBinding(const std::string& objectName,
		const std::string& spanName, bool driveStateFromCamera);

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// singleton
	static Camera3DEditor* GetInstance();
	static void Finalize();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	static Camera3DEditor* instance_;
	SceneView* sceneView_;

	// カメラ調整項目データ
	std::unordered_map<std::string, CameraPathData> params_;
	// 対象アクション
	std::unordered_map<std::string, CameraPathController::ActionSynchBind> actionBinds_;

	// カメラの補間処理、更新
	std::unique_ptr<CameraPathController> controller_;

	// エディター
	std::unique_ptr<Camera3DEditorPanel> panel_;       // UIの表示
	std::unique_ptr<CameraPathGizmoSynch> gizmoSynch_; // ギズモ同期
	std::unique_ptr<CameraPathRenderer> renderer_;     // デバッグ表示の線描画

	std::string selectedObjectKey_;   // 選択中のオブジェクト
	std::string selectedActionName_;  // オブジェクトが所持しているアクション
	std::string selectedParamKey_;  // 操作対象のカメラ
	int selectedKeyIndex_ = 0;      // 選択中のキーフレーム
	CameraPathController::PlaybackState playbackCamera_;
	JsonSaveState paramSaveState_;
	char lastLoaded_[128] = {};

	//--------- functions ----------------------------------------------------

	// helper
	float ComputeEffectiveCameraT(const CameraPathController::PlaybackState& state,
		const CameraPathData& data) const;

	Camera3DEditor() :IGameEditor("Camera3DEditor") {}
	~Camera3DEditor() = default;
	Camera3DEditor(const Camera3DEditor&) = delete;
	Camera3DEditor& operator=(const Camera3DEditor&) = delete;
};