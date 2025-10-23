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
//	汎用アルゴリズム(列挙→配列化／文字列処理／探索／補間／範囲判定)を提供する。
//============================================================================
namespace Algorithm {

	//========================================================================
	//	Enum
	//========================================================================

	// クラス名整形時の先頭大文字/小文字/無加工の指定を行う
	enum class LeadingCase {

		AsIs,  // 変更しない
		Lower, // 先頭を小文字にする
		Upper  // 先頭を大文字にする
	};

	// 列挙の0..(enumValue-1)をuint32配列として取得する
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

	// inputからtoRemoveをすべて取り除いた文字列を返す
	std::string RemoveSubstring(const std::string& input, const std::string& toRemove);

	// '_'以降を切り捨てた文字列を返す
	std::string RemoveAfterUnderscore(const std::string& input);

	// labelにindexを連結した識別名を返す
	std::string GetIndexLabel(const std::string& label, uint32_t index);

	// 実行環境に応じて型名をデマングルして返す
	std::string DemangleType(const char* name);
	//	先頭文字の大文字/小文字/無加工を指定どおりに整形する
	std::string AdjustLeadingCase(std::string string, LeadingCase leadingCase);

	// 型Tの名前を取得し、必要なら先頭の大文字/小文字を調整して返す
	template <typename T>
	std::string ClassName(LeadingCase mode = LeadingCase::AsIs) {

		std::string name = DemangleType(typeid(T).name());
		name = RemoveSubstring(name, "class ");
		name = RemoveSubstring(name, "struct ");
		name = RemoveSubstring(name, "enum ");
		return Algorithm::AdjustLeadingCase(std::move(name), mode);
	}

	// UTF-8のstd::stringをstd::wstringへ変換する
	std::wstring ConvertString(const std::string& str);

	// ワイド文字列を小文字化して返す
	std::wstring ToLowerW(std::wstring s);

	// ワイド文字列が指定サフィックスで終わるか判定する
	bool EndsWithW(const std::wstring& s, const std::wstring& suf);

	//========================================================================
	//	Find
	//========================================================================

	// メンバfindを持つ連想系コンテナでキーの存在を判定する（必要に応じASSERT）
	template <typename, typename = std::void_t<>>
	struct has_find_method : std::false_type {};
	template <typename T>
	struct has_find_method<T, std::void_t<decltype(std::declval<T>().find(std::declval<typename T::key_type>()))>>
		: std::true_type {
	};
	template <typename T>
	constexpr bool has_find_method_v = has_find_method<T>::value;

	// 連想コンテナに対しkeyの存在を返す（assertionEnable時に未発見ならASSERT）
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
	// シーケンスコンテナに対しkeyの存在を返す（assertionEnable時に未発見ならASSERT）
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

	// 非整数型Tの線形補間値を返す
	template <typename T, std::enable_if_t<!std::is_integral_v<T>, int> = 0>
	inline T Lerp(const T& start, const T& end, float t) {
		return start + (end - start) * t;
	}
	// 整数型Tの線形補間値を四捨五入して返す
	template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
	inline T Lerp(const T& start, const T& end, float t) {

		float v = static_cast<float>(start) +
			(static_cast<float>(end) - static_cast<float>(start)) * t;
		return static_cast<T>(std::lround(v));
	}
	// int専用の線形補間(四捨五入)を返す
	int LerpInt(int a, int b, float t);

	// 値を[0,1]にクランプする
	float Clamp(float value);
	// 全体進捗overallを[start,end]に正規化して[0,1]で返す
	float MapOverallToLocal(float overall, float start, float end);
	// overallが[start,end]の閉区間内にあるか判定する
	bool InRangeOverall(float overall, float start, float end);
}