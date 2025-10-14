#include "Vector3.h"

//============================================================================*/
//	include
//============================================================================*/
#include <Engine/Asset/AssetStructure.h>
#include <Engine/Core/Debug/Assert.h>

//============================================================================*/
//	Vector3 classMethods
//============================================================================*/

Vector3 Vector3::operator+(const Vector3& other) const {
	return Vector3(x + other.x, y + other.y, z + other.z);
}
Vector3 Vector3::operator-(const Vector3& other) const {
	return Vector3(x - other.x, y - other.y, z - other.z);
}
Vector3 Vector3::operator*(const Vector3& other) const {
	return Vector3(x * other.x, y * other.y, z * other.z);
}
Vector3 Vector3::operator/(const Vector3& other) const {
	return Vector3(x / other.x, y / other.y, z / other.z);
}

Vector3& Vector3::operator+=(const Vector3& v) {
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}
Vector3& Vector3::operator-=(const Vector3& v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

Vector3& Vector3::operator*=(const Vector3& v) {
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

Vector3 Vector3::operator-(float scalar) const {
	return Vector3(x - scalar, y - scalar, z - scalar);
}
Vector3 operator-(float scalar, const Vector3& v) {
	return Vector3(v.x - scalar, v.y - scalar, v.z - scalar);
}
Vector3 Vector3::operator*(float scalar) const {
	return Vector3(x * scalar, y * scalar, z * scalar);
}
Vector3 operator*(float scalar, const Vector3& v) {
	return Vector3(v.x * scalar, v.y * scalar, v.z * scalar);
}
Vector3 Vector3::operator/(float scalar) const {
	return Vector3(x / scalar, y / scalar, z / scalar);
}
Vector3 operator/(float scalar, const Vector3& v) {
	return Vector3(v.x / scalar, v.y / scalar, v.z / scalar);
}

Vector3& Vector3::operator*=(float scalar) {
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}
Vector3& Vector3::operator/=(float scalar) {
	x /= scalar;
	y /= scalar;
	z /= scalar;
	return *this;
}

Vector3 Vector3::operator-() const {

	return Vector3(-x, -y, -z);
}

bool Vector3::operator==(const Vector3& other) const {
	return x == other.x && y == other.y && z == other.z;
}
bool Vector3::operator!=(const Vector3& other) const {
	return !(*this == other);
}

bool Vector3::operator>=(const Vector3& other) const {
	return this->Length() >= other.Length();
}

bool Vector3::operator<=(const Vector3& other) const {
	return this->Length() <= other.Length();
}

Json Vector3::ToJson() const {
	return Json{ {"x", x}, {"y", y}, {"z", z} };
}

Vector3 Vector3::FromJson(const Json& data) {
	Vector3 v{};
	if (data.is_array() && data.size() == 3) {
		v.x = data[0].get<float>();
		v.y = data[2].get<float>();
		v.z = data[1].get<float>();
	} else if (data.contains("x") && data.contains("y") && data.contains("z")) {
		v.x = data["x"].get<float>();
		v.y = data["y"].get<float>();
		v.z = data["z"].get<float>();
	}
	return v;
}

void Vector3::Init() {
	this->x = 0.0f;
	this->y = 0.0f;
	this->z = 0.0f;
}

float Vector3::Length() const {
	return std::sqrtf(x * x + y * y + z * z);
}

Vector3 Vector3::Normalize() const {
	float length = this->Length();
	if (length == 0.0f) {
		return Vector3(0.0f, 0.0f, 0.0f);
	}
	return Vector3(x / length, y / length, z / length);
}

Vector3 Vector3::AnyInit(float value) {

	return Vector3(value, value, value);
}

Vector3 Vector3::RotateVector(const Vector3& baseDirection, float angleRad) {

	float cosAngle = std::cos(angleRad);
	float sinAngle = std::sin(angleRad);

	// XZ平面上での回転
	return Vector3(
		baseDirection.x * cosAngle - baseDirection.z * sinAngle,
		baseDirection.y, // Yはそのまま
		baseDirection.x * sinAngle + baseDirection.z * cosAngle
	);
}

Vector3 Vector3::Cross(const Vector3& v0, const Vector3& v1) {
	return {
	  v0.y * v1.z - v0.z * v1.y,
	  v0.z * v1.x - v0.x * v1.z,
	  v0.x * v1.y - v0.y * v1.x };
}

float Vector3::Dot(const Vector3& v0, const Vector3& v1) {
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

float Vector3::Length(const Vector3& v) {

	return std::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector3 Vector3::Normalize(const Vector3& v) {

	float length = Length(v);
	if (length == 0.0f) {
		return Vector3(0.0f, 0.0f, 0.0f);
	}
	return Vector3(v.x / length, v.y / length, v.z / length);
}

Vector3 Vector3::CalculateValue(const std::vector<Keyframe<Vector3>>& keyframes, float time) {

	// キーが1つか、時刻がキーフレーム前なら最初の値とする
	if (keyframes.size() == 1 || time <= keyframes[0].time) {

		return keyframes[0].value;
	}

	for (size_t index = 0; index < keyframes.size(); index++) {

		size_t nextIndex = index + 1;

		// indexとnextIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {

			// 範囲内を補完する
			float t =
				(time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);

			return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}

	// ここまで来た場合は1番後の時刻よりも後ろなので最後の値を返す
	return (*keyframes.rbegin()).value;
}

Vector3 Vector3::Lerp(const Vector3& v0, const Vector3& v1, float t) {
	return Vector3(std::lerp(v0.x, v1.x, t), std::lerp(v0.y, v1.y, t), std::lerp(v0.z, v1.z, t));
}

Vector3 Vector3::Reflect(const Vector3& input, const Vector3& normal) {
	float dotProduct = Dot(input, normal);
	return input - normal * (2.0f * dotProduct);
}

Vector3 Vector3::Transform(const Vector3& v, const Matrix4x4& matrix) {

	Vector3 result;

	result.x = v.x * matrix.m[0][0] + v.y * matrix.m[1][0] + v.z * matrix.m[2][0] +
		matrix.m[3][0];
	result.y = v.x * matrix.m[0][1] + v.y * matrix.m[1][1] + v.z * matrix.m[2][1] +
		matrix.m[3][1];
	result.z = v.x * matrix.m[0][2] + v.y * matrix.m[1][2] + v.z * matrix.m[2][2] +
		matrix.m[3][2];
	float w = v.x * matrix.m[0][3] + v.y * matrix.m[1][3] + v.z * matrix.m[2][3] +
		matrix.m[3][3];

	if (w != 0.0f) {
		result.x /= w;
		result.y /= w;
		result.z /= w;
	}

	return result;
}

Vector3 Vector3::TransferNormal(const Vector3& v, const Matrix4x4& m) {

	Vector3 vector{};

	vector.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0];
	vector.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1];
	vector.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2];

	return vector;
}