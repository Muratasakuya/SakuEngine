#pragma once

//============================================================================
//	include
//============================================================================

// spdlog
#ifndef SPDLOG_FUNCTION
#define SPDLOG_FUNCTION __FUNCSIG__
#endif
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#if defined(_MSC_VER)
#include <spdlog/sinks/msvc_sink.h>
#endif
#include <spdlog/sinks/basic_file_sink.h>
// c++
#include <filesystem>
#include <vector>
#include <chrono>
#include <string>

//============================================================================
//	SpdLogger class
//	コンソール/ファイル(MSVC)への複数シンク出力をまとめるロガー。初期化と各種ログAPIを提供する。
//============================================================================
class SpdLogger {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SpdLogger() = default;
	~SpdLogger() = default;

	// ログレベル種別
	enum class LogLevel {
		INFO,
		ASSERT_ERROR
	};

	// ロガー(エンジン用)を初期化。出力先とフォーマットを設定
	static void Init(const std::string& fileName = "engine.log", bool truncate = true);
	// アセット監視用ロガーを初期化。asset.logへ出力
	static void InitAsset(const std::string& fileName = "assetCheck.log", bool truncate = true);
	// 文字列を指定レベルで出力
	static void Log(const std::string& message, LogLevel level = LogLevel::INFO);

	// 書式つきでログ出力(printf風/型安全)
	template <typename... Args>
	static void LogFormat(LogLevel level, fmt::format_string<Args...> fmt, Args&&... args);

	//--------- accessor -----------------------------------------------------

	// 内部spdlogロガーへの参照を取得
	static std::shared_ptr<spdlog::logger>& Get() { return logger_; }
	static std::shared_ptr<spdlog::logger>& GetAsset() { return assetLogger_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	static inline std::shared_ptr<spdlog::logger> logger_;
	static inline std::shared_ptr<spdlog::logger> assetLogger_;
};

//============================================================================
//	SpdLogger templateMethods
//============================================================================

template<typename ...Args>
inline void SpdLogger::LogFormat(LogLevel level, fmt::format_string<Args...> fmt, Args && ...args) {

	switch (level) {
	case LogLevel::INFO:

		logger_->log(spdlog::level::info, fmt, std::forward<Args>(args)...);
		break;
	case LogLevel::ASSERT_ERROR:

		logger_->log(spdlog::level::critical, fmt, std::forward<Args>(args)...);
		break;
	}
}

//============================================================================
//	SpdLogger defines
//============================================================================

#define LOG_INFO(...)  SPDLOG_LOGGER_CALL(SpdLogger::Get(), spdlog::level::info,     __VA_ARGS__)
#define LOG_WARN(...)  SPDLOG_LOGGER_CALL(SpdLogger::Get(), spdlog::level::warn,     __VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_LOGGER_CALL(SpdLogger::Get(), spdlog::level::err,      __VA_ARGS__)
#define LOG_CRIT(...)  SPDLOG_LOGGER_CALL(SpdLogger::Get(), spdlog::level::critical, __VA_ARGS__)

#define LOG_ASSET_INFO(...)  SPDLOG_LOGGER_CALL(SpdLogger::GetAsset(), spdlog::level::info, __VA_ARGS__)

//============================================================================
//	ScopedMsLog class
//	スコープ滞在時間をmsで自動記録するRAIIタイマ。スコープ終了時にINFO出力する。
//============================================================================
class ScopedMsLog final {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	// ラベル名を付けて計測開始
	explicit ScopedMsLog(std::string label) :
		label_(std::move(label)), start_(std::chrono::steady_clock::now()) {}

	// デストラクタで経過時間を計算しロギング
	~ScopedMsLog() {

		using namespace std::chrono;
		const auto us = duration_cast<microseconds>(steady_clock::now() - start_).count();
		const double ms = static_cast<double>(us) / 1000.0;
		SpdLogger::Get()->log(spdlog::level::info, "[TIMER] {} : {:.3f} ms", label_, ms);
	}
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	std::string label_;
	std::chrono::steady_clock::time_point start_;
};

//============================================================================
//	ScopedMsLog defines
//============================================================================
#ifndef SPDLOG_FUNCTION
#   define SPDLOG_FUNCTION __FUNCSIG__
#endif
#define CONCAT_INNER(a,b) a##b
#define CONCAT(a,b) CONCAT_INNER(a,b)
#define LOG_SCOPE_MS_THIS() ::ScopedMsLog CONCAT(_scopems_, __LINE__)(SPDLOG_FUNCTION)
#define LOG_SCOPE_MS_LABEL(label) ::ScopedMsLog CONCAT(_scopems_, __LINE__)(label)