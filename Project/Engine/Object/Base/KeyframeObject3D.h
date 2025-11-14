#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>
#include <Engine/Utility/Animation/LerpKeyframe.h>
#include <Engine/Utility/Enum/AnyMoldEnum.h>

//============================================================================
//	KeyframeObject3D class
//	キーフレーム補間オブジェクト
//============================================================================
class KeyframeObject3D {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 任意の型、追加できるのはこれだけ
	using AnyValue = std::variant<float, Vector2, Vector3, Color>;
public:
	//========================================================================
	//	public Methods
	//========================================================================

	KeyframeObject3D() = default;
	~KeyframeObject3D() = default;

	// 初期化
	void Init(const std::string& name, const std::string& modelName = "defaultCube");

	// KeyframeObject3Dの時間で更新
	void SelfUpdate();
	// 外部からの値入力による更新
	void ExternalInputTUpdate(float inputT);

	// エディター
	void ImGui();

	// json
	void FromJson(const Json& data);
	void ToJson(Json& data);

	// 補間開始
	void StartLerp();

	// 補間処理するキーの値を追加
	void AddKeyValue(AnyMold mold, const std::string& name);

	//--------- accessor -----------------------------------------------------

	// 再生中かどうか
	bool IsUpdating() const { return currentState_ == State::Updating; }

	// 指定インデックス番目のトランスフォームを返す
	const Transform3D& GetIndexTransform(uint32_t index) const;
	// 指定インデックス番目の任意の型の現在の値を返す
	AnyValue GetIndexAnyValue(uint32_t index, const std::string& name) const;

	// 現在のトランスフォームを返す
	const Transform3D& GetCurrentTransform() const { return currentTransform_; }
	// 追加されている任意の型の現在の値を返す
	AnyValue GetCurrentAnyValue(const std::string& name) const;

	// キーオブジェクトの全てのIDを返す、これはエンジン内のオブジェクトID
	std::vector<uint32_t> GetKeyObjectIDs() const;
	// エンジン内のオブジェクトIDから何番目のキーか取得
	uint32_t GetKeyIndexFromObjectID(uint32_t index);
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

	// 任意値トラック情報
	struct AnyTrack {

		AnyMold type;     // 型のEnum
		std::string name; // ImGuiに出す名前
	};

	// キー情報
	struct Key {

		Transform3D transform; // トランスフォーム
		float time;            // 時間
		EasingType easeType;   // イージング

		// 任意な型の値
		std::vector<AnyValue> anyValues;
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// キーフレーム表示用オブジェクトの名前
	std::string keyObjectName_;
	std::string keyModelName_;
	const std::string keyGroupName_ = "KeyObject";

	// 表示オブジェクト、シーンにしか表示しない
	std::vector<std::unique_ptr<GameObject3D>> keyObjects_;

	// 親トランスフォーム、回転と座標
	std::string parentName_;
	Transform3D* parent_ = nullptr;

	// キー情報
	std::vector<Key> keys_;
	// 任意値トラック情報のリスト
	std::vector<AnyTrack> anyTracks_;

	// 現在のトランスフォーム
	Transform3D currentTransform_;
	// 現在の任意な型の値
	std::vector<AnyValue> currentAnyValues_;

	// 補間
	LerpKeyframe::Type lerpType_; // 補間タイプ
	float timer_;        // 現在の経過時間
	bool isConnectEnds_; // 最初と最後のキーを結ぶかどうか

	// エディター
	float addKeyTimeStep_; // キーを追加するときの時間差分
	bool isDrawKeyframe_;  // キーフレーム表示
	bool isEditUpdate_;    // ImGui関数内で更新するかどうか

	//--------- functions ----------------------------------------------------

	// キーオブジェクトの作成
	std::unique_ptr<GameObject3D> CreateKeyObject(const Transform3D& transform);

	// キートランスフォームの取得
	std::vector<Vector3> GetScales() const;
	std::vector<Quaternion> GetRotations() const;
	std::vector<Vector3> GetPositions() const;
	// 各区間の補間t取得
	float GetT(float currentT) const;

	// 任意値の更新
	void UpdateAnyValues(float currentT);

	// 任意値のデフォルト値
	AnyValue MakeDefaultAnyValue(AnyMold mold);

	// 任意の型の補間後の値取得
	template<typename T>
	T GetLerpedAnyValue(uint32_t trackIndex, float currentT) const;

	// キータイムラインの描画
	void DrawKeyTimeline();

	// キー位置、線の描画
	void DrawKeyLine();
};

//============================================================================
//	KeyframeObject3D templateMethods
//============================================================================

template<typename T>
inline T KeyframeObject3D::GetLerpedAnyValue(uint32_t trackIndex, float currentT) const {

	std::vector<T> values;
	values.reserve(keys_.size());
	for (const auto& key : keys_) {
		if (trackIndex < key.anyValues.size()) {
			if (auto* anyValues = std::get_if<T>(&key.anyValues[trackIndex])) {

				values.emplace_back(*anyValues);
			}
		}
	}
	T result = LerpKeyframe::GetValue<T>(values, currentT, lerpType_);
	return result;
}