#include "MathUtils.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Random/RandomGenerator.h>
#include <Engine/Scene/Camera/BaseCamera.h>
#include <Engine/Config.h>

//============================================================================
//	MathUtils namespaceMethods
//============================================================================

float Math::AbsFloat(float v) {

	return v < 0.0f ? -v : v;
}

float Math::GetYawRadian(const Vector3& direction) {

	return std::atan2(direction.z, direction.x);
}

Vector3 Math::RandomPointOnArc(const Vector3& center,
	const Vector3& direction, float radius, float halfAngle) {

	const float baseYaw = GetYawRadian(direction.Normalize());
	const float halfRad = pi * halfAngle / 180.0f;
	const float randYaw = RandomGenerator::Generate(-halfRad, halfRad);
	const float yaw = baseYaw + randYaw;

	return { center.x + radius * std::cos(yaw),center.y,center.z + radius * std::sin(yaw) };
}

Vector3 Math::RandomPointOnArcInSquare(const Vector3& arcCenter, const Vector3& direction,
	float radius, float halfAngle, const Vector3& squareCenter,
	float clampHalfSize, int tryCount) {

	const float baseYaw = GetYawRadian(direction.Normalize());
	const float halfRad = pi * halfAngle / 180.0f;

	// 正方形の境界
	const float minX = squareCenter.x - clampHalfSize;
	const float maxX = squareCenter.x + clampHalfSize;
	const float minZ = squareCenter.z - clampHalfSize;
	const float maxZ = squareCenter.z + clampHalfSize;

	for (int i = 0; i < tryCount; ++i) {

		const float randYaw = RandomGenerator::Generate(-halfRad, halfRad);
		const float yaw = baseYaw + randYaw;

		Vector3 dir{ std::cos(yaw), 0.0f, std::sin(yaw) };

		// その方向でx,z境界に当たるまでの距離
		const float tX = std::abs(dir.x) < 1e-6f ?
			FLT_MAX : (dir.x > 0 ? (maxX - arcCenter.x) / dir.x
				: (minX - arcCenter.x) / dir.x);
		const float tZ = std::abs(dir.z) < 1e-6f ?
			FLT_MAX : (dir.z > 0 ? (maxZ - arcCenter.z) / dir.z :
				(minZ - arcCenter.z) / dir.z);

		// 納まる座標があれば設定
		const float rMaxDir = (std::max)(0.0f, (std::min)(tX, tZ));
		if (radius <= rMaxDir) {

			return arcCenter + dir * radius;
		}
	}

	// 見つからない場合、一番近い距離にする
	return {
		std::clamp(arcCenter.x, minX, maxX),
		arcCenter.y,
		std::clamp(arcCenter.z, minZ, maxZ) };
}

Vector3 Math::RotateY(const Vector3& v, float rad) {

	Matrix4x4 rotate = Matrix4x4::MakeYawMatrix(rad);
	return Vector3::Transform(v, rotate).Normalize();
}

void Math::ToColumnMajor(const Matrix4x4& matrix, float out[16]) {

	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {

			out[c * 4 + r] = matrix.m[r][c];
		}
	}
}

void Math::FromColumnMajor(const float in[16], Matrix4x4& matrix) {

	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {

			matrix.m[r][c] = in[c * 4 + r];
		}
	}
}

Vector2 Math::ProjectToScreen(const Vector3& translation, const BaseCamera& camera) {

	Matrix4x4 viewMatrix = camera.GetViewMatrix();
	Matrix4x4 projectionMatrix = camera.GetProjectionMatrix();

	Vector3 viewPos = Vector3::Transform(translation, viewMatrix);
	Vector3 clipPos = Vector3::Transform(viewPos, projectionMatrix);

	float screenX = (clipPos.x * 0.5f + 0.5f) * Config::kWindowWidthf;
	float screenY = (1.0f - (clipPos.y * 0.5f + 0.5f)) * Config::kWindowHeightf;

	return Vector2(screenX, screenY);
}