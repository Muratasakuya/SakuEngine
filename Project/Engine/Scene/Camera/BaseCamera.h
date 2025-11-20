#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/Transform.h>
#include <Engine/Utility/Timer/StateTimer.h>

// imgui
#include <imgui.h>

//============================================================================
//	BaseCamera class
//	カメラの基底クラス、継承しなくてもこのクラスを使ってカメラを実装できる
//============================================================================
class BaseCamera {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 更新のモード
	enum class UpdateMode {

		Euler,
		Quaternion
	};
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BaseCamera();
	virtual ~BaseCamera() = default;

	virtual void Init() {};

	virtual void Update() {};
	// ビュープロジェクション行列の更新
	void UpdateView(UpdateMode updateMode = UpdateMode::Euler);

	// 共通エディター
	virtual void ImGui();
	// フラスタムの編集
	void EditFrustum();
	void RenderFrustum();

	// 自動フォーカス開始
	void StartAutoFocus(bool isFocus, const Vector3& target);

	//--------- accessor -----------------------------------------------------

	// 親の設定、特定のキーにのみ設定
	void SetEditorParentTransform(const std::string& keyName, const Transform3D& parent);

	void SetParent(const Transform3D* parent) { transform_.parent = parent; };
	void SetTranslation(const Vector3& translation) { transform_.translation = translation; }
	void SetRotation(const Quaternion& rotation) { transform_.rotation = rotation; }
	void SetEulerRotation(const Vector3& eulerRotation) { transform_.eulerRotate = eulerRotation; }
	void SetFovY(float fovY) { fovY_ = fovY; }
	void SetIsUpdateEditor(bool isUpdateEditor) { isUpdateEditor_ = isUpdateEditor; }

	float GetFovY() const { return fovY_; }
	float GetNearClip() const { return nearClip_; }
	float GetFarClip() const { return farClip_; }
	bool IsUpdateDebugView() const { return updateDebugView_; }
	bool IsUpdateEditor() const { return isUpdateEditor_; }

	const Transform3D& GetTransform() const { return transform_; }

	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
	const Matrix4x4& GetBillboardMatrix() const { return billboardMatrix_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const float itemWidth_ = 224.0f;

	float fovY_;
	float nearClip_;
	float farClip_;
	float aspectRatio_;

	Transform3D transform_;

	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 viewProjectionMatrix_;
	Matrix4x4 billboardMatrix_;

	// 選択したオブジェクトへの自動フォーカス
	bool isStartFocus_ = false;
	StateTimer autoFucusTimer_;
	Vector3 startFocusTranslation_;
	Vector3 targetFocusTranslation_;
	Quaternion startFocusRotation_;
	Quaternion targetFocusRotation_;

	// debug
	float frustumScale_;
	bool displayFrustum_;
	bool updateDebugView_;
	bool isUpdateEditor_ = false;

	//--------- functions ----------------------------------------------------

	// 自動フォーカス更新
	void UpdateAutoFocus();

	// ビルボード行列計算
	void CalBillboardMatrix();
};