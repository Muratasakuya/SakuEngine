#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <vector>
// json
#include <Externals/nlohmann/json.hpp>
// using
using Json = nlohmann::json;

//============================================================================
//	Vector2 class
//============================================================================
class Vector2 final {
public:

	float x, y;

	Vector2() : x(0.0f), y(0.0f) {}
	Vector2(float x, float y) : x(x), y(y) {}

	//--------- operators ----------------------------------------------------

	Vector2 operator+(const Vector2& other) const;
	Vector2 operator-(const Vector2& other) const;
	Vector2 operator*(const Vector2& other) const;
	Vector2 operator/(const Vector2& other) const;

	Vector2& operator+=(const Vector2& v);
	Vector2& operator-=(const Vector2& v);
	Vector2& operator*=(const Vector2& v);
	Vector2& operator/=(const Vector2& v);

	Vector2 operator*(float scalar) const;
	friend Vector2 operator*(float scalar, const Vector2& v);

	Vector2 operator/(float scalar) const;
	friend Vector2 operator/(float scalar, const Vector2& v);

	Vector2& operator*=(float scalar);

	bool operator==(const Vector2& other) const;
	bool operator!=(const Vector2& other) const;

	bool operator<=(const Vector2& other) const;
	bool operator>=(const Vector2& other) const;

	//----------- json -------------------------------------------------------

	Json ToJson() const;
	static Vector2 FromJson(const Json& data);

	//--------- functions ----------------------------------------------------

	void Init();

	float Length() const;

	Vector2 Normalize() const;

	static Vector2 AnyInit(float value);

	static float Length(const Vector2& v);

	static Vector2 Normalize(const Vector2& v);

	static Vector2 Lerp(const Vector2& v0, const Vector2& v1, float t);

	// catmullRom
	static Vector2 CatmullRomInterpolation(const Vector2& p0, const Vector2& p1,
		const Vector2& p2, const Vector2& p3, float t);
	static Vector2 CatmullRomValue(const std::vector<Vector2>& points, float t);
};