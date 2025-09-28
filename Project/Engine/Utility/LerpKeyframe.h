#pragma once

//============================================================================
//	include
//============================================================================
#include <Lib/MathUtils/MathUtils.h>

//============================================================================
//	LerpKeyframe
//============================================================================

namespace LerpKeyframe {

	// 補間の仕方
	enum class Type {

		None,   // 通常
		Bezier, // ベジェ曲線
		Spline, // スプライン曲線
	};

	//============================================================================
	//	CommonFunctions
	//============================================================================

	template <class T>
	concept Lerpable =
		requires (const T & a, const T & b, float s) {
			{ a + b } -> std::same_as<T>;
			{ a - b } -> std::same_as<T>;
			{ a* s } -> std::same_as<T>;
			{ s* a } -> std::same_as<T>;
	};

	template <Lerpable T>
	inline T Multiply(const T& v, float s) {
		if constexpr (requires { v* s; }) {

			return v * s;
		} else {

			return s * v;
		}
	}

	template <Lerpable T>
	inline T Lerp(const T& a, const T& b, float t) {

		return a + LerpKeyframe::Multiply((b - a), t);
	}

	//============================================================================
	//	Spline
	//============================================================================

	template <Lerpable T>
	inline T CatmullRomInterpolation(const T& p0, const T& p1, const T& p2, const T& p3, float t) {

		const float t2 = t * t;
		const float t3 = t2 * t;

		T e0 = (LerpKeyframe::Multiply(p1, 2.0f));
		T e1 = (-p0 + p2);
		T e2 = (LerpKeyframe::Multiply(p0, 2.0f) - LerpKeyframe::Multiply(p1, 5.0f) + LerpKeyframe::Multiply(p2, 4.0f) - p3);
		T e3 = (LerpKeyframe::Multiply(p1, 3.0f) - LerpKeyframe::Multiply(p2, 3.0f) + p3 - p0);

		T result = LerpKeyframe::Multiply(e3, t3) + LerpKeyframe::Multiply(e2, t2) + LerpKeyframe::Multiply(e1, t) + e0;
		return LerpKeyframe::Multiply(result, 0.5f);
	}

	template <Lerpable T>
	inline T CatmullRomValue(const std::vector<T>& points, float t) {

		// 存在しない場合は初期値を返す
		if (points.empty()) {

			return T();
		}
		// 4点以上あるとき
		if (4 <= points.size()) {

			const size_t division = points.size() - 1;
			const float  area = 1.0f / static_cast<float>(division);
			float localT = std::clamp(std::fmod((std::max)(t, 0.0f), 1.0f) / area - std::floor(t / area), 0.0f, 1.0f);
			size_t index = static_cast<size_t>(t / area);
			index = (std::min)(index, division - 1);

			size_t i0 = (index == 0) ? index : index - 1;
			size_t i1 = index;
			size_t i2 = (std::min)(index + 1, division);
			size_t i3 = (std::min)(index + 2, division);

			return LerpKeyframe::CatmullRomInterpolation(points[i0], points[i1], points[i2], points[i3], localT);
		}
		// 1点の時
		else if (points.size() == 1) {

			return points.front();
		}
		// 2点の時
		else if (points.size() == 2) {

			return LerpKeyframe::Lerp(points[0], points[1], std::clamp(t, 0.0f, 1.0f));
		}
		// 3点の時
		else if (points.size() == 3) {

			// 0-1-2の区間で2つの線形で補間
			const float mid = 0.5f;
			if (t <= mid) {

				return LerpKeyframe::Lerp(points[0], points[1], t / mid);
			} else {

				return LerpKeyframe::Lerp(points[1], points[2], (t - mid) / (1.0f - mid));
			}
		}
		return points.front();
	}

	//============================================================================
	//	Bezier
	//============================================================================

	template <Lerpable T>
	inline T BezierValue(const T& p0, const T& p1, const T& p2, const T& p3, float t) {

		const float u = 1.0f - t;
		const float u2 = u * u;
		const float u3 = u2 * u;
		const float t2 = t * t;
		const float t3 = t2 * t;
		return LerpKeyframe::Multiply(p0, u3) + LerpKeyframe::Multiply(p1, 3.0f * u2 * t) +
			LerpKeyframe::Multiply(p2, 3.0f * u * t2) + LerpKeyframe::Multiply(p3, t3);
	}

	//============================================================================
	//	enumから補間値の取得
	//============================================================================

	template <Lerpable T>
	inline T GetValue(const std::vector<T>& points, float t, Type type) {

		// 何も値がなければ初期値を返す
		if (points.empty()) {
			return T{};
		}
		switch (type) {
		case Type::None: {

			// 通常の補間
			if (points.size() == 1) {
				return points[0];
			}
			const size_t division = points.size() - 1;
			const float  area = 1.0f / static_cast<float>(division);
			size_t index = static_cast<size_t>(t / area);
			index = (std::min)(index, division - 1);
			const float localT = (t - index * area) / area;
			return LerpKeyframe::Lerp(points[index], points[index + 1], localT);
		}
		case Type::Spline:

			// スプライン曲線
			return LerpKeyframe::CatmullRomValue(points, t);
		case Type::Bezier: {

			// ベジェ曲線
			if (points.size() < 4) {
				if (points.size() == 1) {
					return points[0];
				}
				if (points.size() == 2) {
					return Lerp(points[0], points[1], t);
				}
				// p3とp2を同値として補間させる
				return LerpKeyframe::BezierValue(points[0], points[1], points[2], points[2], t);
			}
			return LerpKeyframe::BezierValue(points[0], points[1], points[2], points[3], t);
		}
		}
		return points.front();
	}

	//============================================================================
	//	補間値の平均取得
	//============================================================================

	// 補間値の平均化
	template <typename T>
	inline std::vector<float> AveragingPoints(const std::vector<T>& points, int division, Type type) {

		std::vector<float> averagedT{};

		// 最初の値は0.0fになる
		averagedT.emplace_back(0.0f);
		// 合計
		float totalLength = 0.0f;
		for (int index = 0; index < division; ++index) {

			// 現在のt
			float t0 = static_cast<float>(index) / division;
			// 次のt
			float t1 = static_cast<float>(index + 1) / division;

			// それぞれの値を求める
			T p1 = LerpKeyframe::GetValue<T>(points, t0, type);
			T p2 = LerpKeyframe::GetValue<T>(points, t1, type);

			float segmentLength = 0.0f;
			if constexpr (std::is_same_v<T, float>) {

				segmentLength = std::sqrtf(p2 - p1);
			} else {

				segmentLength = T::Length(p2 - p1);
			}

			totalLength += segmentLength;
			averagedT.emplace_back(totalLength);
		}
		for (auto& length : averagedT) {

			length /= totalLength;
		}
		averagedT.back() = 1.0f;
		return averagedT;
	}
	float GetReparameterizedT(float t, const std::vector<float>& averagedT);
}