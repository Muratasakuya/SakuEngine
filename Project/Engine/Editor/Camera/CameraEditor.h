#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>
#include <Engine/Object/Base/KeyframeObject3D.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

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

	// 初期化
	void Init(SceneView* sceneView);

	// 更新
	void Update();

	// エディター
	void ImGui() override;

	// 読み込んでデータを作成、jsonBasePath_ + (...ここ)
	// isInEditorはエディター内での読み込みかどうか
	void LoadJson(const std::string& fileName, bool isInEditor = false);

	// アニメーション処理
	// 開始呼び出し
	void StartAnim(const std::string& keyName, bool isAddFirstKey = true);
	// 現在アクティブなアニメーションの終了呼び出し
	void EndAnim();

	// 現在アクティブなアニメーションが終了したか
	bool IsAnimFinished() const;

	//--------- accessor -----------------------------------------------------

	// singleton
	static CameraEditor* GetInstance();
	static void Finalize();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// ゲームカメラに反映させるときの表示モード
	enum class PreviewMode {

		Keyframe, // 操作中のキーフレームの視点
		Manual,   // 手動で動かした補間値(0.0f~1.0f)の視点
		Play      // 再生
	};

	//--------- variables ----------------------------------------------------

	static CameraEditor* instance_;
	SceneView* sceneView_;

	// 保存するファイルパス
	const std::string jsonBasePath_ = "CameraEditor/";

	// 表示するキーオブジェクトのモデル
	const std::string keyObjectName_ = "cameraEditKey";
	const std::string keyModelName_ = "demoCamera";
	// 追加するキー情報
	const std::string addKeyValueFov_ = "FovY"; // 画角

	// キーオブジェクト、補間の値
	// std::stringがキーの名前
	std::unordered_map<std::string, std::unique_ptr<KeyframeObject3D>> keyObjects_;

	// ゲームで開始呼びだししたアクティブなキーオブジェクト
	KeyframeObject3D* activeKeyObject_ = nullptr;

	// エディター
	std::string selectedKeyObjectName_; // 選択されているキーオブジェクトの名前
	JsonSaveState jsonSaveState_;       // json保存状態
	// 名前の累計カウント、重複しないようにするため
	std::unordered_map<std::string, int32_t> nameCounts_;

	// ゲームカメラとの連携
	bool isPreViewGameCamera_ = false;                // ゲームカメラへ調整結果を反映させるか
	PreviewMode previewMode_ = PreviewMode::Keyframe; // 表示方法
	// PreviewModeごとの値
	// Keyframe
	int32_t previewKeyIndex_ = -1; // 表示するキー位置のインデックス
	// Manual
	float previewTimer_; // 現在の補間進捗(0.0f~1.0f)
	// Play
	float previewLoopSpacing_ = 1.0f; // ループ間隔
	float previewLoopTimer_ = 0.0f;   // ループ管理経過時間

	//--------- functions ----------------------------------------------------

	// 保存
	void SaveJson(const std::string& fileName);

	// キーオブジェクトの更新
	void UpdateKeyObjects();
	// エディター内の更新
	void UpdateEditor();

	// エディター
	// キーオブジェクトの追加、選択
	void AddAndSelectKeyObjectMap();
	// 選択したキーオブジェクトの編集
	void EditSelectedKeyObject();

	// カメラへの適応
	void ApplyToCamera(BaseCamera& camera, const KeyframeObject3D& keyObject);
	// 値操作中のキーインデックスの同期
	void SynchSelectedKeyIndex();

	// 名前の重複チェックと修正
	std::string CheckName(const std::string& name);
	// 名前からベースネームと番号を分離する
	std::string SplitBaseNameAndNumber(const std::string& name, int& number);

	CameraEditor() :IGameEditor("CameraEditor") {}
	~CameraEditor() = default;
	CameraEditor(const CameraEditor&) = delete;
	CameraEditor& operator=(const CameraEditor&) = delete;
};