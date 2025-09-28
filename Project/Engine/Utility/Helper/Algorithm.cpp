#include "Algorithm.h"

//============================================================================
//	Algorithm classMethods
//============================================================================

std::string Algorithm::RemoveSubstring(const std::string& input, const std::string& toRemove) {

	std::string result = input;
	size_t pos;

	// toRemoveが見つかる限り削除する
	while ((pos = result.find(toRemove)) != std::string::npos) {
		result.erase(pos, toRemove.length());
	}

	return result;
}

std::string Algorithm::RemoveAfterUnderscore(const std::string& input) {

	size_t pos = input.find('_');
	if (pos != std::string::npos) {
		return input.substr(0, pos);
	}
	return input;
}

std::string Algorithm::GetIndexLabel(const std::string& label, uint32_t index) {

	return label + std::to_string(index);
}

std::wstring Algorithm::ConvertString(const std::string& str) {

	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

int Algorithm::LerpInt(int a, int b, float t) {

	float v = static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * t;
	// 四捨五入して返す
	return static_cast<int>(std::lround(v));
}