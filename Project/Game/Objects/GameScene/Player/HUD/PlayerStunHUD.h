#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/Base/GameTimerDisplay.h>

// c++
#include <array>

//============================================================================
//	PlayerStunHUD class
//	敵をスタンさせたときに表示するHUD
//============================================================================
class PlayerStunHUD {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerStunHUD() = default;
	~PlayerStunHUD() = default;

	// 初期化
	void Init();
	
	// スタンしているときのみ更新
	void Update();

	// エディター
	void ImGui();

	//--------- accessor -----------------------------------------------------

	// 有効かどうか
	void SetVaild();
	void SetCancel();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	struct ChainInput {

		std::unique_ptr<GameObject2D> rightChain; // 切り替え入力: 右
		std::unique_ptr<GameObject2D> leftChain;  // 切り替え入力: 左
		std::unique_ptr<GameObject2D> cancel;     // キャンセル入力

		void Init(const std::string& rightTex, const std::string& leftTex,
			const std::string& cancelTex);

		void SetSize(const Vector2& size);
	};

	enum class State {

		Begin,  // 最初のanimation
		Count,  // 秒数経過開始
		Cancel, // キャンセルを押したか、時間が経過したとき
	};

	//--------- variables ----------------------------------------------------

	State currentState_;

	std::unique_ptr<GameTimerDisplay> restTimerDisplay_; // 時間表示

	// 切り替え先アイコン表示
	static const uint32_t iconCount_ = 2;
	std::array<std::unique_ptr<GameObject2D>, iconCount_> stunChainIcon_;
	// アイコンの周りの円
	std::array<std::unique_ptr<GameObject2D>, iconCount_>  stunChainIconRing_;

	// 入力表示
	ChainInput keyInput_;     // キー表示
	ChainInput gamepadInput_; // パッド表示

	std::unique_ptr<GameObject2D> progressBarBackground_; // 経過率背景
	std::unique_ptr<GameObject2D> progressBar_;           // 経過率
	std::unique_ptr<GameObject2D> chainAttackText_;       // 文字

	float restTimer_; // 時間経過
	float restTime_;  // スタン選択時間

	float beginAnimationTimer_; // 最初のanimationの経過時間
	float beginAnimationTime_;  // 最初のanimationの時間
	int beginAlphaBlinkingCount_; // 時間内のalpha点滅回数
	int restAlphaBlinkingCount_;  // 時間内のalpha点滅回数
	EasingType beginAnimationEasingType_;

	float cancelTimer_; // キャンセル時間経過
	float cancelTime_;  // キャンセル時間
	float cancelUnderOffsetY_; // 下に下げていく量
	EasingType cancelEasingType_;

	// parameters
	// 座標
	const float centerTranslationX_ = 960.0f;
	float timerTranslationX_;  // タイマーのX座標
	float barTranslationY_;    // バーのY座標
	float timerTranslationY_;  // タイマーのY座標
	float cancelTranslationY_; // キャンセルのY座標
	float endOffsetX_;         // 両端のオフセットX
	float timerOffsetX_;       // タイマーの間の間隔X

	// サイズ
	Vector2 iconSize_;     // アイコンのサイズ
	Vector2 iconRingSize_; // アイコンリングのサイズ
	Vector2 progressBarBackgroundSize_; // 経過率の背景サイズ
	Vector2 progressBarSize_;           // 経過率のサイズ
	Vector2 chainAttackTextSize_;       // 文字サイズ
	Vector2 chainInputSize_;  // 入力キーのサイズ
	Vector2 cancelInputSize_; // キャンセルのサイズ
	Vector2 timerSize_;       // タイマーのサイズ
	Vector2 timerSymbolSize_; // タイマーの記号のサイズ

	// 色
	Color iconRingColor_;
	float iconRingEmissive_;

	// test
	bool isVaild_;         // 有効かどうか
	bool isCountFinished_; // カウントが終了したかどうか

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// update
	void UpdateLayout();
	void UpdateSize();
	void UpdateState();
	void UpdateBeginAnimation();
	void UpdateCount();
	void UpdateCancel();

	// helper
	void SetSize(const Vector2& size);
	void SetTargetSize(float lerpT);
	void SetTargetTranslation(float lerpT);
	void SetAlpha(float alpha);
	float CalcBlinkAlpha(float progress, int blinkCount);
};