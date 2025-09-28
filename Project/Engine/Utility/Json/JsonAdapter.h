#pragma once

//============================================================================*/
//	include
//============================================================================*/
#include <Engine/MathLib/Vector2.h>
#include <Engine/MathLib/Vector3.h>
#include <Engine/MathLib/Vector4.h>
#include <Engine/MathLib/Quaternion.h>

// c++
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>
#include <array>
#include <cassert>

//============================================================================*/
//	JsonAdapter class
//============================================================================*/
class JsonAdapter {
public:
	//========================================================================*/
	//	public Methods
	//========================================================================*/

	JsonAdapter() = default;
	~JsonAdapter() = default;

	// 保存と読み込み
	static void Save(const std::string& saveDirectoryFilePath, const Json& jsonData);
	static Json Load(const std::string& loadDirectoryFilePath);
	static bool LoadAssert(const std::string& loadDirectoryFilePath);
	static bool LoadCheck(const std::string& loadDirectoryFilePath, Json& data);

	// value
	template <typename T>
	static T GetValue(const Json& data, const std::string& key);

	// object
	template <typename T>
	static Json FromObject(const T& obj);
	template <typename T>
	static T ToObject(const Json& data);

	// vector
	template <typename T>
	static Json FromVector(const std::vector<T>& vec);
	template <typename T>
	static std::vector<T> ToVector(const Json& data);

	// array
	template <typename T, std::size_t N>
	static Json FromArray(const std::array<T, N>& arr);
	template <typename T, std::size_t N>
	static std::array<T, N> ToArray(const Json& data);

	//========================================================================*/
	//* variables

	static const std::string& baseDirectoryFilePath_;
};

//============================================================================*/
//	JsonAdapter templateMethods
//============================================================================*/

template<typename T>
inline T JsonAdapter::GetValue(const Json& data, const std::string& key) {

	if (data.contains(key)) {

		return data.at(key).get<T>();
	}

	// 存在しない場合
	return T{};
}

template<typename T>
inline Json JsonAdapter::FromObject(const T& obj) {

	return obj.ToJson();
}

template<typename T>
inline T JsonAdapter::ToObject(const Json& data) {

	return T::FromJson(data);
}

template <typename T>
inline Json JsonAdapter::FromVector(const std::vector<T>& vec) {

	Json jsonArray = Json::array();
	for (const auto& element : vec) {
		jsonArray.push_back(element);
	}
	return jsonArray;
}

template <typename T>
inline std::vector<T> JsonAdapter::ToVector(const Json& data) {

	std::vector<T> vec;
	if (data.is_array()) {
		for (const auto& element : data) {
			vec.push_back(element.get<T>());
		}
	}
	return vec;
}

template <typename T, std::size_t N>
inline Json JsonAdapter::FromArray(const std::array<T, N>& arr) {

	Json jsonArray = Json::array();
	for (const auto& element : arr) {
		jsonArray.push_back(element);
	}
	return jsonArray;
}

template <typename T, std::size_t N>
inline std::array<T, N> JsonAdapter::ToArray(const Json& data) {

	std::array<T, N> arr{};
	if (data.is_array() && data.size() == N) {
		for (std::size_t i = 0; i < N; ++i) {
			arr[i] = data[i].get<T>();
		}
	}
	return arr;
}