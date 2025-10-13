#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <functional>
#include <string>
#include <vector>

//============================================================================
//	CurveValueEditor class
//============================================================================
class CurveValueEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	// 操作を行えるカーブを登録
	void Registry(void* key, std::function<bool()> draw);

	// カーブを開く
	void Open(void* key);

	// 選択の変更
	bool ConsumeChanged(void* key);

	// 現在アクティブ状態のカーブを操作する
	void Edit();

	//--------- accessor -----------------------------------------------------

	// singleton
	static CurveValueEditor* GetInstance();
	static void Finalize();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 登録されたカーブデータ
	struct Entry {

		void* key = nullptr;        // 識別子
		std::function<bool()> draw; // タブの描画関数
		bool changed = false;       // 変更フラグ
	};

	//--------- variables ----------------------------------------------------

	static CurveValueEditor* instance_;

	// 登録されたカーブデータ
	std::vector<Entry> entries_;
	bool showWindow_ = true;
	int  activeIndex_ = -1;

	//--------- functions ----------------------------------------------------

	// helper
	int FindIndex(void* key) const;

	CurveValueEditor() = default;
	~CurveValueEditor() = default;
	CurveValueEditor(const CurveValueEditor&) = delete;
	CurveValueEditor& operator=(const CurveValueEditor&) = delete;
};