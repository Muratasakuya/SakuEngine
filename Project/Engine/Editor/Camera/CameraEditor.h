#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>
#include <Engine/Object/Base/KeyframeObject3D.h>

// front
class SceneView;

//============================================================================
//	CameraEditor class
//	カメラのキーフレーム制御を行い反映させるエディター
//============================================================================
class CameraEditor :
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	void Init(SceneView* sceneView);

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// singleton
	static CameraEditor* GetInstance();
	static void Finalize();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	static CameraEditor* instance_;
	SceneView* sceneView_;

	// 表示するキーオブジェクトのモデル
	const std::string keyObjectName_ = "cameraEditKey";
	const std::string keyModelName_ = "demoCamera";

	// キーオブジェクト、補間の値
	// std::stringがキーの名前
	std::unordered_map<std::string, std::unique_ptr<KeyframeObject3D>> keyObjects_;

	// エディター
	std::string selectedKeyObjectName_; // 選択されているキーオブジェクトの名前

	//--------- functions ----------------------------------------------------

	// エディター
	// キーオブジェクトの追加、選択
	void AddAndSelectKeyObjectMap();
	// 選択したキーオブジェクトの編集
	void EditSelectedKeyObject();

	CameraEditor() :IGameEditor("CameraEditor") {}
	~CameraEditor() = default;
	CameraEditor(const CameraEditor&) = delete;
	CameraEditor& operator=(const CameraEditor&) = delete;
};