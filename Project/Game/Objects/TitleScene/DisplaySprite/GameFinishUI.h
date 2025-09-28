#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/Base/GameButton.h>
#include <Engine/Utility/Animation/SimpleAnimation.h>

//============================================================================
//	GameFinishUI class
//============================================================================
class GameFinishUI {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameFinishUI() = default;
	~GameFinishUI() = default;

	void Init();

	void Update();

	// editor
	void ImGui();

	// json
	void ApplyJson(const Json& data);
	void SaveJson(Json& data);

	//--------- accessor -----------------------------------------------------

	// ゲーム終了フラグ
	bool IsSelectFinish() const;
	bool IsSelectState() const { return currentState_ == State::Select; }
	bool IsSelectMenuReady() const { return currentState_ == State::Select && currentSelectState_ == SelectState::Select; }

	GameButton* GetPowerButton() const { return powerIcon_.get(); }
	GameButton* GetCancelButton() const { return selectCancel_.get(); }
	GameButton* GetOKButton() const { return selectOK_.get(); }
	// パッド操作入力設定
	void ConfirmPowerByPad() { currentState_ = State::Select; }
	void ConfirmCancelByPad();
	void ConfirmOKByPad();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		Power,  // 電源ボタン表示
		Select, // 終了するかキャンセルか
		Finish  // 終了
	};
	enum class SelectState {

		Begin,  // 最初のアニメーション
		Select, // 選択中
		Decide  // 決定後
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;
	SelectState currentSelectState_;

	// 表示するスプライト
	// 常に表示
	std::unique_ptr<GameButton> powerIcon_; // 電源アイコン
	Vector2 powerIconSize_;

	// 状態に応じて表示
	std::unique_ptr<GameObject2D> askFinish_;           // 終了しますか表示
	std::unique_ptr<GameObject2D> askFinishBackground_; // 終了しますか表示の背景

	std::unique_ptr<GameButton> selectCancel_; // キャンセル
	std::unique_ptr<GameButton> selectOK_;     // OK -> Finish

	// parameters
	float selectButtonSpacing_; // ボタンの間の距離
	// animations
	std::unique_ptr<SimpleAnimation<Vector2>> finishSizeAnimation_;
	std::unique_ptr<SimpleAnimation<Vector2>> finishBackgroundSizeAnimation_;
	std::unique_ptr<SimpleAnimation<Vector2>> selectCancelSizeAnimation_;
	std::unique_ptr<SimpleAnimation<Vector2>> selectOKSizeAnimation_;

	//--------- functions ----------------------------------------------------

	// init
	void InitSprites();
	void InitAnimations();
	void InitAnimationSize();
	void SetSpritePos();

	// update
	void CheckSelect();
	void UpdateState();
	// select
	void UpdateSelect();
	void CheckGameFinish();
	void DisableSelectSprites();

	// helper
	void UpdateEnableCollision();
	void LerpSelectSpriteSize();
	void StartSizeAnimations();
	bool IsFinishedAllAnimations();
	void ImGuiSize();

	template <typename T>
	void ForEachAnimations(T function);
};

//============================================================================
//	GameFinishUI templateMethods
//============================================================================

template<typename T>
inline void GameFinishUI::ForEachAnimations(T function) {

	function(finishSizeAnimation_.get());
	function(finishBackgroundSizeAnimation_.get());
	function(selectCancelSizeAnimation_.get());
	function(selectOKSizeAnimation_.get());
}