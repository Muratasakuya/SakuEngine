#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>
#include <Game/Objects/Base/GamecButtonFocusNavigator.h>
#include <Game/Objects/TitleScene/DisplaySprite/GameFinishUI.h>

//============================================================================
//	TitleDisplaySprite class
//	タイトルシーンで表示するスプライト
//============================================================================
class TitleDisplaySprite :
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	TitleDisplaySprite() :IGameEditor("TitleDisplaySprite") {}
	~TitleDisplaySprite() = default;

	// 初期化
	void Init();

	// 更新
	void Update();

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// ゲーム終了フラグ
	bool IsSelectFinish() const { return finishUI_->IsSelectFinish(); }

	// ゲーム遷移フラグ
	bool IsGameStart() const { return isGameStart_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// ゲーム開始フラグ
	bool isGameStart_;

	// 表示するスプライト
	std::unique_ptr<GameObject2D> name_; // 名前
	std::unique_ptr<GameButton> start_; // 開始文字
	std::unique_ptr<GameObject2D> operateDevice_; // 操作デバイス

	std::unique_ptr<GameFinishUI> finishUI_; // 終了表示

	// 入力管理
	std::unique_ptr<GamecButtonFocusNavigator> buttonFocusNavigator_;
	ButtonFocusGroup currentFocusGroup_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// init
	void InitSprites();
	void InitNavigator();
	void SetSpritePos();
	
	// update
	void UpdateInputGamepad();

	// helper
	void CheckGameStart();
};