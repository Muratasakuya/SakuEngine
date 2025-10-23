#pragma once

//============================================================================
//	include
//============================================================================

// windows
#include <Windows.h>
// c++
#include <crtdbg.h>
#include <iostream>
#include <string>
#include <cassert>
#include <sstream>

//============================================================================
//	Assert class
//	デバッグ/リリースで動作を切り替えるアサートユーティリティ。ログ出力と停止/終了を行う。
//============================================================================
class Assert {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Assert() = default;
	~Assert() = default;

	//--------- functions ----------------------------------------------------

	// デバッグ時: 条件NGならメッセージを出力しデバッガ停止(_ASSERT_EXPR)
	static void DebugAssert(bool condition, const std::string& message, const char* function);
	// リリース時: 条件NGならクリティカルログを出力しプロセス終了
	static void ReleaseAssert(bool condition, const std::string& message, const char* function);
	// ビルド設定に応じてDebug/Releaseアサートを振り分けて実行
	static void AssertHandler(bool condition, const std::string& message, const char* function);
};

//============================================================================
//	macro definition
//============================================================================
#define ASSERT(condition, message) Assert::AssertHandler(condition, message, __FUNCTION__)