#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>

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

	void Init(const std::string& name);

	// エディター
	void ImGui();

	// json
	void FromJson(const Json& data);
	void ToJson(Json& data);

	//--------- accessor -----------------------------------------------------

	// キー位置を返す
	const std::vector<Vector3>& GetKeyPositions() const { return keyPositions_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// キーフレーム表示用オブジェクトの名前
	std::string keyObjectName_;
	const std::string keyModelName_ = "debugSphere";
	const std::string keyGroupName_ = "KeyObject";

	// 表示オブジェクト、シーンにしか表示しない
	std::vector<std::unique_ptr<GameObject3D>> keyObjects_;

	// 親トランスフォーム、回転と座標
	std::string parentName_;
	const Transform3D* parent_ = nullptr;

	// キー位置
	std::vector<Vector3> keyPositions_;

	//--------- functions ----------------------------------------------------

	// キーオブジェクトの作成
	std::unique_ptr<GameObject3D> CreateKeyObject(const Vector3 & pos);
};