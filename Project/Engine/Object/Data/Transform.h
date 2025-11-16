#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/GPUObject/CBufferStructures.h>
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>
#include <Engine/MathLib/Vector2.h>
#include <Engine/MathLib/Vector3.h>
#include <Engine/MathLib/Quaternion.h>

// c++
#include <format>

//============================================================================
//	BaseTransform class
//	3DTransformの基底クラス
//============================================================================
class BaseTransform {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BaseTransform() = default;
	virtual ~BaseTransform() = default;

	// 初期化
	void Init();

	// 行列更新
	void UpdateMatrix();

	// エディター
	void ImGui(float itemSize);

	// json
	void ToJson(Json& data);
	void FromJson(const Json& data);

	//--------- accessor -----------------------------------------------------

	// matrixからワールド座標、向きを取得
	Vector3 GetWorldScale() const;       // ワールドスケール
	Quaternion GetWorldRotation() const; // ワールド回転
	Vector3 GetWorldPos() const;         // ワールド座標

	Vector3 GetForward() const;

	Vector3 GetBack() const;

	Vector3 GetRight() const;

	Vector3 GetLeft() const;

	Vector3 GetUp() const;

	Vector3 GetDown() const;

	// 変更があったかどうか
	bool IsDirty() const { return isDirty_; }
	void SetIsDirty(bool isDirty);

	//--------- variables ----------------------------------------------------

	// 拡縮
	Vector3 scale;

	// 回転
	Quaternion rotation;
	Vector3 eulerRotate;

	// 座標
	Vector3 translation;
	Vector3 offsetTranslation;

	TransformationMatrix matrix;
	const BaseTransform* parent = nullptr;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Vector3 prevScale;
	Quaternion prevRotation;
	Vector3 prevTranslation;
	Vector3 prevOffsetTranslation;

	// 変更があったかどうかのフラグ
	bool isDirty_;
};

//============================================================================
//	Transform3D class
//	3DTransformクラス
//============================================================================
class Transform3D :
	public BaseTransform {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Transform3D() = default;
	~Transform3D() = default;

	//--------- accessor -----------------------------------------------------

	void SetInstancingName(const std::string& name) { meshInstancingName_ = name; }
	const std::string& GetInstancingName() const { return meshInstancingName_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// meshInstancing用の名前
	std::string meshInstancingName_;
};

//============================================================================
//	EffectTransform class
//	エフェクト用Transform
//============================================================================
struct EffectTransform {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	void Init();

	void ImGui(float itemSize);

	//--------- variables ----------------------------------------------------

	// 回転
	Quaternion rotation;
	Vector3 eulerRotate;

	// 座標
	Vector3 translation;
};

//============================================================================
//	Transform2D class
//	2DTransformクラス
//============================================================================
class Transform2D {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Transform2D() = default;
	~Transform2D() = default;

	void Init(ID3D12Device* device);

	void UpdateMatrix();

	void ImGui(float itemSize, float buttonSize = 32.0f);

	// json
	void ToJson(Json& data);
	void FromJson(const Json& data);

	//--------- accessor -----------------------------------------------------

	// 画面の中心に設定
	void SetCenterPos();

	//--------- variables ----------------------------------------------------

	Vector2 translation;
	float rotation;

	Vector2 size;           // 表示サイズ
	Vector2 sizeScale;      // スケール
	Vector2 anchorPoint;    // アンカーポイント

	Vector2 textureLeftTop; // テクスチャ左上座標
	Vector2 textureSize;    // テクスチャ切り出しサイズ

	// 0: 左下
	// 1: 左上
	// 2: 右下
	// 3: 右上
	std::array<Vector2, 4> vertexOffset_; // 頂点オフセット

	Matrix4x4 matrix;

	const Transform2D* parent = nullptr;

	// 親がいてもその場で回転するかどうか
	bool rotateAroundSelfWhenParented = true;

	//--------- accessor -----------------------------------------------------

	const DxConstBuffer<Matrix4x4>& GetBuffer() const { return buffer_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// buffer
	DxConstBuffer<Matrix4x4> buffer_;
};