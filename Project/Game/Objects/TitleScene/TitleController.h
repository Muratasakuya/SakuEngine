#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>

//============================================================================
//	TitleController class
//	タイトルシーンのスプライト表示など、遷移までの管理
//============================================================================
class TitleController :
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	TitleController() :IGameEditor("TitleController") {}
	~TitleController() = default;

	// 初期化
	void Init();

	// 更新、遷移判定もここで行っている
	void Update();

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// ゲーム終了フラグ
	bool IsSelectFinish() const { return isSelectFinish_; }

	// ゲーム遷移フラグ
	bool IsGameStart() const { return isGameStart_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// デバッグ用フラグ
	bool isGameStart_ = false;
	bool isSelectFinish_ = false;
};