#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <string>
#include <filesystem>

//============================================================================
//	FileSystem namespace
//	資産探索ユーティリティ。ベースパス以下を再帰走査し、名前/拡張子でファイルを見つける。
//============================================================================
namespace Filesystem {

	//--------- functions ----------------------------------------------------

	// basePath配下を再帰探索し、完全一致するファイル名を見つけてfullPathへ返す
	bool Found(const std::filesystem::path& basePath, const std::string& fileName,
		std::filesystem::path& fullPath);

	// 指定stemと拡張子群に合致する最初のファイルを探し、パスを返す
	bool FindByStem(const std::filesystem::path& basePath,
		const std::string& stem, const std::vector<std::string>& extensions,
		std::filesystem::path& outPath);
};