#pragma once

//============================================================================
// include
//============================================================================
#include <Engine/MathLib/Vector2.h>
#include <Engine/MathLib/Vector3.h>
#include <Engine/MathLib/Vector4.h>
#include <Engine/MathLib/Quaternion.h>
#include <Engine/MathLib/Matrix4x4.h>
#include <Engine/Utility/Enum/Direction.h>

// c++
#include <numbers>
#include <vector>
#include <algorithm>
// front
class BaseCamera;

constexpr float pi = std::numbers::pi_v<float>;
constexpr float radian = pi / 180.0f;

//============================================================================
// Math namespace
// 汎用的な数学関数群
//============================================================================

namespace Math {

	// 軸
	enum class Axis {

		X,
		Y,
		Z
	};

	// 絶対値を返す
	float AbsFloat(float v);

	// 方向ベクトルからyaw角(ラジアン)を算出する
	float GetYawRadian(const Vector3& direction);

	// 角度を[-π,π]範囲に折り返す
	float WrapDegree(float value);
	float WrapPi(float value);

	// from→toのヨー最短方向を{-1,0,+1}で返す
	int YawShortestDirection(const Quaternion& from, const Quaternion& to);

	// from→toのヨー角の符号付き差(ラジアン)を返す
	float YawSignedDelta(const Quaternion& from, const Quaternion& to);

	// ツイストQuaternionから指定軸の角度(ラジアン)を算出する
	float AngleFromTwist(const Quaternion& twist, Axis axis);

	// 円弧上のランダム点を生成する
	Vector3 RandomPointOnArc(const Vector3& center, const Vector3& direction,
		float radius, float halfAngle);

	// 正方形領域に収まる円弧上のランダム点を生成する
	Vector3 RandomPointOnArcInSquare(const Vector3& arcCenter, const Vector3& direction,
		float radius, float halfAngle, const Vector3& squareCenter,
		float clampHalfSize, int tryCount = 12);

	// Y軸回りにベクトルを回転させる
	Vector3 RotateY(const Vector3& v, float rad);

	// 行列を列メジャー配列へ書き出す
	void ToColumnMajor(const Matrix4x4& matrix, float out[16]);

	// 列メジャー配列から行列へ読み込む
	void FromColumnMajor(const float in[16], Matrix4x4& matrix);

	// ワールド座標をスクリーン座標へ射影する
	Vector2 ProjectToScreen(const Vector3& translation, const BaseCamera& camera);

	// Quaternionから指定方向ベクトルを取得する
	Vector3 QuaternionToDirection(const Quaternion& rotation, Direction3D direction);
}