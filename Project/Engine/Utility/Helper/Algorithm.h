#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>

// c++
#include <cstdint>
#include <vector>
#include <utility>
#include <algorithm>
#include <any>
#include <locale>
#include <iostream>

//============================================================================
//	Algorithm namespace
//============================================================================
namespace Algorithm {

	//========================================================================
	//	Enum
	//========================================================================

	// クラスの名前取得時の設定
	enum class LeadingCase {

		AsIs,  // 変更しない
		Lower, // 先頭を小文字にする
		Upper  // 先頭を大文字にする
	};

	template <typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
	std::vector<uint32_t> GetEnumArray(Enum enumValue) {

		std::vector<uint32_t> intValues;
		for (uint32_t i = 0; i < static_cast<uint32_t>(enumValue); ++i) {

			intValues.push_back(i);
		}
		return intValues;
	}

	//========================================================================
	//	String
	//========================================================================

	std::string RemoveSubstring(const std::string& input, const std::string& toRemove);

	std::string RemoveAfterUnderscore(const std::string& input);

	std::string GetIndexLabel(const std::string& label, uint32_t index);

	std::string DemangleType(const char* name);
	std::string AdjustLeadingCase(std::string string, LeadingCase leadingCase);

	template <typename T>
	std::string ClassName(LeadingCase mode = LeadingCase::AsIs) {

		std::string name = DemangleType(typeid(T).name());
		name = RemoveSubstring(name, "class ");
		name = RemoveSubstring(name, "struct ");
		name = RemoveSubstring(name, "enum ");
		return Algorithm::AdjustLeadingCase(std::move(name), mode);
	}

	std::wstring ConvertString(const std::string& str);

	std::wstring ToLowerW(std::wstring s);

	bool EndsWithW(const std::wstring& s, const std::wstring& suf);

	//========================================================================
	//	Find
	//========================================================================

	template <typename, typename = std::void_t<>>
	struct has_find_method : std::false_type {};
	template <typename T>
	struct has_find_method<T, std::void_t<decltype(std::declval<T>().find(std::declval<typename T::key_type>()))>>
		: std::true_type {
	};
	template <typename T>
	constexpr bool has_find_method_v = has_find_method<T>::value;

	template <typename TA, typename TB>
	typename std::enable_if_t<has_find_method_v<TA>, bool>
		Find(const TA& object, const TB& key, bool assertionEnable = false) {

		auto it = object.find(key);
		bool found = it != object.end();

		if (!found && assertionEnable) {
			ASSERT(false, "not found this object");
		}
		return found;
	}
	template <typename TA, typename TB>
	typename std::enable_if_t<!has_find_method_v<TA>, bool>
		Find(const TA& object, const TB& key, bool assertionEnable = false) {

		auto it = std::find(object.begin(), object.end(), key);
		bool found = it != object.end();

		if (!found && assertionEnable) {
			ASSERT(false, "not found this object");
		}

		return found;
	}

	//========================================================================
	// Math
	//========================================================================

	// 線形補間
	template <typename T, std::enable_if_t<!std::is_integral_v<T>, int> = 0>
	inline T Lerp(const T& start, const T& end, float t) {
		return start + (end - start) * t;
	}
	template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
	inline T Lerp(const T& start, const T& end, float t) {

		float v = static_cast<float>(start) +
			(static_cast<float>(end) - static_cast<float>(start)) * t;
		return static_cast<T>(std::lround(v));
	}
	int LerpInt(int a, int b, float t);

	// 値制御
	float Clamp(float value);
	// 全体の進捗から開始、終了地点の補間値を取得
	float MapOverallToLocal(float overall, float start, float end);
	// 全体進捗区間内か
	bool InRangeOverall(float overall, float start, float end);
}