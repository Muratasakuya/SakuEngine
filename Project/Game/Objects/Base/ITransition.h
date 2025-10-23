#pragma once

//============================================================================
//	include
//============================================================================

//imgui
#include <imgui.h>

// 遷移状態
enum class TransitionState {

	Begin,   // 開始
	Load,    // 読み込み中
	LoadEnd, // 読み込み終了
	End      // 終了
};

//============================================================================
//	ITransition class
//	シーン遷移インターフェース
//============================================================================
class ITransition {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ITransition() = default;
	virtual ~ITransition() = default;

	// 初期化
	virtual void Init() = 0;

	// 更新処理
	virtual void Update() = 0;
	
	// 遷移入りの更新
	virtual void BeginUpdate() = 0;

	// 読み込み中(NowLoading...など)の更新
	virtual void LoadUpdate() = 0;
	virtual void LoadEndUpdate() = 0;

	// 遷移終わりの更新
	virtual void EndUpdate() = 0;

	// エディター
	virtual void ImGui() = 0;

	//--------- accessor -----------------------------------------------------

	void SetLoadingFinished(bool finished) { loadingFinished_ = finished; }

	TransitionState GetState() const { return state_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 遷移状態
	TransitionState state_;

	// 読み込みが終了したかどうか
	bool loadingFinished_;
};