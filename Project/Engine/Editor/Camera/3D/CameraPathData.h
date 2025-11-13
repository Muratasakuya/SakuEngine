#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>
#include <Engine/Utility/Animation/LerpKeyframe.h>

//============================================================================
//	CameraPathData class
//	カメラ補間データ
//============================================================================
class CameraPathData {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	CameraPathData() = default;
	~CameraPathData() = default;

	//--------- structure ----------------------------------------------------

	struct KeyframeParam {

		KeyframeParam() = default;
		~KeyframeParam() = default;

		float fovY;          // 画角
		Vector3 translation; // 座標の保持
		Quaternion rotation; // 回転の保持

		// 場所、回転を可視化するオブジェクト
		std::unique_ptr<GameObject3D> demoObject;
		Vector3 viewScale;

		// 停止時間
		float stayTime = 0.0f;

		// init
		void Init(bool isUseGame);

		// json
		void FromJson(const Json& data);
		void ToJson(Json& data);

		// コピー禁止
		KeyframeParam(const KeyframeParam&) = delete;
		KeyframeParam& operator=(const KeyframeParam&) = delete;
		// ムーブ許可
		KeyframeParam(KeyframeParam&&) noexcept = default;
		KeyframeParam& operator=(KeyframeParam&&) noexcept = default;
	};

	//--------- variables ----------------------------------------------------

	static inline const std::string demoCameraJsonPath = "CameraEditor/demoCamera.json";
	static inline const std::string cameraParamJsonPath = "CameraEditor/CameraParams/";

	// 対象名
	std::string name; // 名前

	// 追従先の設定
	bool followTarget = false;
	bool followRotation = true;
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

	// 停止
	bool staying = false;    // いま停止中か
	int stayKeyIndex = -1;   // 停止しているキーのインデックス
	float stayRemain = 0.0f; // 残り停止時間
	float lastEasedT = 0.0f; // ひとつ前のeasedT

	// エディター
	bool isUseGame = false;
	bool isDrawKeyframe = false;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson(const std::string& fileName, bool isUseGame);
	void SaveJson(const std::string& fileName);

	//キー座標の配列を取得
	std::vector<Vector3> CollectTranslationPoints() const;
	float UpdateAndGetEffectiveEasedT();

	// コピー禁止
	CameraPathData(const CameraPathData&) = delete;
	CameraPathData& operator=(const CameraPathData&) = delete;
	// ムーブ許可
	CameraPathData(CameraPathData&&) noexcept = default;
	CameraPathData& operator=(CameraPathData&&) noexcept = default;
};