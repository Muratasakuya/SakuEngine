#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	IValueSource class
//	処理(補間など...)した値を取得するためのインターフェース
//============================================================================
template <typename T>
class IValueSource {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IValueSource() = default;
	virtual ~IValueSource() = default;

	// 更新開始
	virtual void Start(const T& base) = 0;

	// 値の更新
	virtual void Update() = 0;

	// 値の取得
	virtual T GetValue() const = 0;

	// 終了判定
	virtual bool IsFinished() const = 0;

	// リセット
	virtual void Reset() = 0;

	// エディター
	virtual void ImGui(const char* label) = 0;

	// json
	virtual void FromJson(const Json& data) = 0;
	virtual void ToJson(Json& data) = 0;
};