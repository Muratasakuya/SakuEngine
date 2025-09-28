#include "JsonAdapter.h"

//============================================================================*/
//	JsonAdapter classMethods
//============================================================================*/

const std::string& JsonAdapter::baseDirectoryFilePath_ = "./Assets/Json/";

void JsonAdapter::Save(const std::string& saveDirectoryFilePath, const Json& jsonData) {

	// フォルダのフルパス取得
	std::string fullPath = baseDirectoryFilePath_ + saveDirectoryFilePath;
	std::filesystem::path path(fullPath);

	if (path.extension() != ".json") {

		path.replace_extension(".json");
		fullPath = path.string();
	}

	if (!std::filesystem::exists(path.parent_path())) {
		std::filesystem::create_directories(path.parent_path());
	}

	std::ofstream file(fullPath);

	if (!file.is_open()) {

		assert(false && "failed to open file for saving json");
		return;
	}

	file << jsonData.dump(4);
	file.close();
}

Json JsonAdapter::Load(const std::string& loadDirectoryFilePath) {

	std::string fullPath = baseDirectoryFilePath_ + loadDirectoryFilePath;
	std::ifstream file(fullPath);

	if (!file.is_open()) {

		// ファイルが存在しないので空のJsonを返す
		return Json();
	}

	Json jsonData;
	file >> jsonData;
	file.close();

	return jsonData;
}

bool JsonAdapter::LoadAssert(const std::string& loadDirectoryFilePath) {

	std::string fullPath = baseDirectoryFilePath_ + loadDirectoryFilePath;
	std::ifstream file(fullPath);

	if (!file.is_open()) {
		return false;
	}

	Json jsonData;
	file >> jsonData;
	file.close();

	return true;
}

bool JsonAdapter::LoadCheck(const std::string& loadDirectoryFilePath, Json& data) {

	std::string fullPath = baseDirectoryFilePath_ + loadDirectoryFilePath;
	std::ifstream file(fullPath);

	if (!file.is_open()) {
		return false;
	}

	file >> data;
	file.close();

	return true;
}