#include "Vector2.h"

//============================================================================*/
//	Vector2 classMethods
//============================================================================*/

Vector2 Vector2::operator+(const Vector2& other) const {
	return Vector2(x + other.x, y + other.y);
}
Vector2 Vector2::operator-(const Vector2& other) const {
	return Vector2(x - other.x, y - other.y);
}
Vector2 Vector2::operator*(const Vector2& other) const {
	return Vector2(x * other.x, y * other.y);
}
Vector2 Vector2::operator/(const Vector2& other) const {
	return Vector2(x / other.x, y / other.y);
}

Vector2& Vector2::operator+=(const Vector2& v) {
	x += v.x;
	y += v.y;
	return *this;
}
Vector2& Vector2::operator-=(const Vector2& v) {
	x += v.x;
	y += v.y;
	return *this;
}

Vector2& Vector2::operator*=(const Vector2& v) {
	x *= v.x;
	y *= v.y;
	return *this;
}

Vector2& Vector2::operator/=(const Vector2& v) {
	x /= v.x;
	y /= v.y;
	return *this;
}

Vector2 Vector2::operator*(float scalar) const {
	return Vector2(x * scalar, y * scalar);
}
Vector2 operator*(float scalar, const Vector2& v) {
	return Vector2(v.x * scalar, v.y * scalar);
}
Vector2 Vector2::operator/(float scalar) const {
	return Vector2(x / scalar, y / scalar);
}
Vector2 operator/(float scalar, const Vector2& v) {
	return Vector2(v.x / scalar, v.y / scalar);
}

Vector2& Vector2::operator*=(float scalar) {
	x *= scalar;
	y *= scalar;
	return *this;
}

bool Vector2::operator==(const Vector2& other) const {
	return x == other.x && y == other.y;
}
bool Vector2::operator!=(const Vector2& other) const {
	return !(*this == other);
}

bool Vector2::operator<=(const Vector2& other) const {
	return this->Length() <= other.Length();
}

bool Vector2::operator>=(const Vector2& other) const {
	return this->Length() >= other.Length();
}

Json Vector2::ToJson() const {
	return Json{ {"x", x}, {"y", y} };
}

Vector2 Vector2::FromJson(const Json& data) {

	Vector2 v{};
	if (data.contains("x") && data.contains("y")) {
		v.x = data["x"].get<float>();
		v.y = data["y"].get<float>();
	}
	return v;
}

void Vector2::Init() {

	this->x = 0.0f;
	this->y = 0.0f;
}

float Vector2::Length() const {
	return std::sqrtf(x * x + y * y);
}

Vector2 Vector2::Normalize() const {
	float length = this->Length();
	if (length == 0.0f) {
		return Vector2(0.0f, 0.0f);
	}
	return Vector2(x / length, y / length);
}

Vector2 Vector2::AnyInit(float value) {

	return Vector2(value, value);
}

float Vector2::Length(const Vector2& v) {
	return std::sqrtf(v.x * v.x + v.y * v.y);
}

Vector2 Vector2::Normalize(const Vector2& v) {

	float length = Vector2::Length(v);
	if (length == 0.0f) {
		return Vector2(0.0f, 0.0f);
	}
	return Vector2(v.x / length, v.y / length);
}

Vector2 Vector2::Lerp(const Vector2& v0, const Vector2& v1, float t) {

	return Vector2(std::lerp(v0.x, v1.x, t), std::lerp(v0.y, v1.y, t));
}

Vector2 Vector2::CatmullRomInterpolation(const Vector2& p0, const Vector2& p1,
	const Vector2& p2, const Vector2& p3, float t) {

	const float s = 0.5f;

	float t2 = t * t;
	float t3 = t2 * t;

	// 各項の計算
	Vector2 e3 = 3.0f * p1 - 3.0f * p2 + p3 - p0;
	Vector2 e2 = 2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3;
	Vector2 e1 = -1.0f * p0 + p2;
	Vector2 e0 = 2.0f * p1;

	// 補間結果の計算
	Vector2 result = t3 * e3 + t2 * e2 + t * e1 + e0;

	return (s * result);
}

Vector2 Vector2::CatmullRomValue(const std::vector<Vector2>& points, float t) {

	assert(points.size() >= 4 && "制御点が足りません");

	// 区間数は制御点の数 - 1
	size_t division = points.size() - 1;
	// 1区間の長さ (全体を 1.0 とした割合)
	float areaWidth = 1.0f / division;

	// 区間内の始点を 0.0f、終点を 1.0f とした時の現在位置
	float t_2 = std::fmod(t, areaWidth) * division;
	// 下限(0.0f)と上限(1.0f)の範囲に収める
	t_2 = std::clamp(t_2, 0.0f, 1.0f);

	// 区間番号
	size_t index = static_cast<size_t>(t / areaWidth);
	// 区間番号が上限を超えないための計算
	index;// ここが分からない

	// 4点分のインデックス
	size_t index0 = index - 1;
	size_t index1 = index;
	size_t index2 = index + 1;
	size_t index3 = index + 2;

	// 最初の区間のp0はp1を重複使用する
	if (index == 0) {

		index0 = index1;
	}
	// 最後の区間のp3はp2を重複使用する
	if (points.size() <= index3) {

		index3 = index2;
	}

	// 4点の座標
	const Vector2& p0 = points[index0];
	const Vector2& p1 = points[index1];
	const Vector2& p2 = points[index2];
	const Vector2& p3 = points[index3];

	return CatmullRomInterpolation(p0, p1, p2, p3, t_2);
}