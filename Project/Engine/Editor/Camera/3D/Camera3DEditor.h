#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/Camera/BaseCamera.h>
#include <Engine/Object/Base/GameObject3D.h>
#include <Engine/Utility/Timer/StateTimer.h>
#include <Engine/Utility/Animation/LerpKeyframe.h>
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

	// 調整を行えるアニメーションの追加
	void AddAnimation(const std::string& name, const SkinnedAnimation* animation);

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// singleton
	static Camera3DEditor* GetInstance();
	static void Finalize();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// キーフレームごとの調整値
	struct KeyframeParam {

		float fovY;          // 画角
		Vector3 translation; // 座標の保持

		// 場所、回転を可視化するオブジェクト
		std::unique_ptr<GameObject3D> demoObject;

		// init
		void Init();

		// json
		void FromJson(const Json& data);
		void ToJson(Json& data);
	};

	// 調整項目をまとめた構造体
	struct CameraParam {

		// 追従先の設定
		bool followTarget;
		const Transform3D* target;
		std::string targetName;

		// キーフレームs
		std::vector<KeyframeParam> keyframes;

		// 補間の仕方
		LerpKeyframe::Type lerpType = LerpKeyframe::Type::Spline;
		// 補間時間
		StateTimer timer;

		bool isDrawLine3D = true;     // デバッグ表示の線を描画するかどうか
		int  divisionCount = 64;      // 曲線の分割数
		bool useAveraging = false;    // 平均化を行うか
		std::vector<float> averagedT; // 平均化されたt値
	};

	//--------- variables ----------------------------------------------------

	static Camera3DEditor* instance_;
	SceneView* sceneView_;

	// jsonのパス
	static inline const std::string demoCameraJsonPath_ = "CameraEditor/demoCamera.json";

	// カメラ調整項目データ
	std::unordered_map<std::string, CameraParam> params_;
	// 対象アニメーション
	std::unordered_map<std::string, const SkinnedAnimation*> skinnedAnimations_;

	// editor
	std::string selectedSkinnedKey_; // 選択中の骨アニメーション
	std::string selectedAnimName_;   // 骨アニメーションが所持しているアニメーション
	std::string selectedParamKey_;   // 操作対象のカメラ
	int selectedKeyIndex_ = 0;       // 選択中のキーフレーム
	bool isDebugViewGameCamera_ = false;

	//--------- functions ----------------------------------------------------

	// json
	void SaveDemoCamera();

	// update
	void UpdateFollowTarget();
	void UpdateDebugViewGameCamera();

	// editor
	void SelectAnimationSubject();
	void AddCameraParam();
	void SelectCameraParam();

	void EditCameraParam();
	void EditLerp(CameraParam& param);
	void EditKeyframe(CameraParam& param);
	void SelectTarget(CameraParam& param);

	// helper
	void SelectKeyframe(const CameraParam& param);
	std::vector<Vector3> CollectTranslationPoints(const CameraParam& param) const;
	void DrawKeyframeLine(const CameraParam& param);
	void SyncSelectedKeyIndexFromGizmo();

	Camera3DEditor() :IGameEditor("Camera3DEditor") {}
	~Camera3DEditor() = default;
	Camera3DEditor(const Camera3DEditor&) = delete;
	Camera3DEditor& operator=(const Camera3DEditor&) = delete;
};