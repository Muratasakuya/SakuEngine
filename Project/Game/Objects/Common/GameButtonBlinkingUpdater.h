#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Animation/AnimationLoop.h>
#include <Game/Objects/Base/IGameButtonResponseUpdater.h>
#include <Engine/Utility/Enum/Easing.h>

//============================================================================
//	GameButtonBlinkingUpdater class
//============================================================================
class GameButtonBlinkingUpdater :
	public IGameButtonResponseUpdater {
public:
	//========================================================================
	//	public Methods
	//==================================================================1======

	GameButtonBlinkingUpdater() = default;
	~GameButtonBlinkingUpdater() = default;

	// 更新開始
	void Begin(GameObject2D& object) override;

	// 更新中
	void ActiveUpdate(GameObject2D& object) override;
	// 非アクティブになったときの更新
	void InactiveUpdate(GameObject2D& object) override;

	// リセット
	void End(GameObject2D& object) override;

	// 常に行う更新処理
	void IdleUpdate(GameObject2D&) override;

	// editor
	void ImGui() override;

	// json
	void FromJson(const Json& data) override;
	void ToJson(Json& data) override;

	//--------- accessor -----------------------------------------------------

	bool IsInactiveFinished() const override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// ループ
	AnimationLoop animationLoop_;

	float loopTimer_;   // 時間経過
	float loopSpacing_; // ループ間隔
	EasingType loopEasing_;

	float endTimer_; // 時間経過
	float endTime_;  // 元に戻っていく時間
	EasingType endEasing_;

	Vector2 baseSize_; // スケーリングを行うサイズ
	float smallScale_; // 最小スケール
	float maxScale_;   // 最大スケール

	Color beginColor_;  // 開始時の色の保持
	Color startColor_;  // 最初の色
	Color targetColor_; // 目標の色
};