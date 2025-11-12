#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>
#include <Engine/Utility/Animation/LerpKeyframe.h>

//============================================================================
//	KeyframeObject3D class
//============================================================================
class KeyframeObject3D {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	KeyframeObject3D() = default;
	~KeyframeObject3D() = default;

	// 初期化
	void Init(const std::string& name);

	// 更新
	void Update();

	// エディター
	void ImGui();

	// json
	void FromJson(const Json& data);
	void ToJson(Json& data);

	// 補間開始
	void StartLerp();

	//--------- accessor -----------------------------------------------------

	// 現在の位置を返す
	const Vector3& GetCurrentPos() const { return currentPos_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 補間状態
	enum class State {

		None,     // 何もしない状態
		Updating, // 補間開始
	};

	// キー情報
	struct Key {

		Vector3 pos; // 位置
		float time;  // 時間
		EasingType easeType; // イージング
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// キーフレーム表示用オブジェクトの名前
	std::string keyObjectName_;
	const std::string keyModelName_ = "debugSphere";
	const std::string keyGroupName_ = "KeyObject";

	// 表示オブジェクト、シーンにしか表示しない
	std::vector<std::unique_ptr<GameObject3D>> keyObjects_;

	// 親トランスフォーム、回転と座標
	std::string parentName_;
	Transform3D* parent_ = nullptr;

	// キー情報
	std::vector<Key> keys_;
	// 現在の位置
	Vector3 currentPos_;

	// 補間
	LerpKeyframe::Type lerpType_; // 補間タイプ
	float timer_;        // 現在の経過時間
	bool isConnectEnds_; // 最初と最後のキーを結ぶかどうか

	// エディター
	bool isDrawKeyframe_; // キーフレーム表示
	bool isEditUpdate_;   // ImGui関数内で更新するかどうか

	//--------- functions ----------------------------------------------------

	// キーオブジェクトの作成
	std::unique_ptr<GameObject3D> CreateKeyObject(const Vector3& pos);

	// キー位置の取得
	std::vector<Vector3> GetPositions() const;
	// 各区間の補間t取得
	float GetT(float currentT) const;

	// キータイムラインの描画
	void DrawKeyTimeline();
};