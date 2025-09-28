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

//============================================================================
//	Algorithm namespace
//============================================================================
namespace Algorithm {

	//========================================================================
	//	Enum
	//========================================================================

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

	template<typename T>
	std::string ClassName(const T& obj) {

		std::string className = typeid(obj).name();
		std::string prefix = "class ";

		return RemoveSubstring(className, prefix);
	}

	std::wstring ConvertString(const std::string& str);

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
	template <typename T>
	inline T Lerp(const T& start, const T& end, float t) {

		return start + (end - start) * t;
	}
	int LerpInt(int a, int b, float t);
}