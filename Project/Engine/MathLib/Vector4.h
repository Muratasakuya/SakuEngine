#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <cmath>
#include <math.h>

// json
#include <Externals/nlohmann/json.hpp>
// using
using Json = nlohmann::json;

//============================================================================
//	Vector4 class
//============================================================================
class Vector4 final {
public:

	float x, y, z, w;

	Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

	//--------- operators ----------------------------------------------------

	Vector4 operator+(const Vector4& other) const;
	Vector4 operator-(const Vector4& other) const;
	Vector4 operator*(const Vector4& other) const;
	Vector4 operator/(const Vector4& other) const;

	Vector4& operator+=(const Vector4& v);
	Vector4& operator-=(const Vector4& v);

	Vector4 operator*(float scalar) const;
	friend Vector4 operator*(float scalar, const Vector4& v);

	Vector4 operator/(float scalar) const;
	friend Vector4 operator/(float scalar, const Vector4& v);

	bool operator==(const Vector4& other) const;
	bool operator!=(const Vector4& other) const;

	//--------- functions ----------------------------------------------------

	void Init();

};

//============================================================================
//	Color class
//============================================================================
class Color final {
public:

	float r, g, b, a;

	Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
	Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

	//----------- json -------------------------------------------------------

	Json ToJson() const;
	static Color FromJson(const Json& data);

	//--------- functions ----------------------------------------------------

	static Color Convert(int color);
	Color ToLinear() const;

	static Color White(float alpha = 1.0f);
	static Color Black(float alpha = 1.0f);

	static	Color Red(float alpha = 1.0f);
	static Color Green(float alpha = 1.0f);
	static Color Blue(float alpha = 1.0f);
	static Color Yellow(float alpha = 1.0f);
	static Color Cyan(float alpha = 1.0f);
	static Color Magenta(float alpha = 1.0f);

	static Color Lerp(const Color& color0, const Color& color1, float t);
};