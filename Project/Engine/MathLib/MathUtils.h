#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector2.h>
#include <Engine/MathLib/Vector3.h>
#include <Engine/MathLib/Vector4.h>
#include <Engine/MathLib/Quaternion.h>
#include <Engine/MathLib/Matrix4x4.h>

// c++
#include <numbers>
#include <vector>
#include <algorithm>
// front
class BaseCamera;

constexpr float pi = std::numbers::pi_v<float>;
constexpr float radian = pi / 180.0f;

//============================================================================
//	Math namespace
//============================================================================

namespace Math {

	float AbsFloat(float v);

	float GetYawRadian(const Vector3& direction);

	Vector3 RandomPointOnArc(const Vector3& center, const Vector3& direction,
		float radius, float halfAngle);

	Vector3 RandomPointOnArcInSquare(const Vector3& arcCenter, const Vector3& direction,
		float radius, float halfAngle, const Vector3& squareCenter,
		float clampHalfSize, int tryCount = 12);

	Vector3 RotateY(const Vector3& v, float rad);

	void ToColumnMajor(const Matrix4x4& matrix, float out[16]);
	void FromColumnMajor(const float in[16], Matrix4x4& matrix);

	Vector2 ProjectToScreen(const Vector3& translation, const BaseCamera& camera);
}