#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>
#include <Engine/Object/Base/GameObject2D.h>
#include <Engine/Utility/Timer/StateTimer.h>

//============================================================================
//	BossEnemyAttackSign class
//============================================================================
class BossEnemyAttackSign :
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyAttackSign() :IGameEditor("BossEnemyAttackSign") {}
	~BossEnemyAttackSign() = default;

	void Init();

	void Update();

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	void Emit(const Vector2& emitPos);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	template <typename T>
	struct LerpValue {

		T start;
		T target;
	};

	struct DirectionInfo {

		Vector2 direction; // 進行方向
		Vector2 anchor;    // アンカーポイント
		bool horizontal;   // 横方向かどうか
	};

	// 状態
	enum class State {

		None,
		Update,
	};

	// 線の方向
	enum class SignDirection {

		Right,  // 右
		Left,   // 左
		Up,     // 上
		Bottom, // 下
		Count
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;

	// 表示スプライト
	std::array<std::unique_ptr<GameObject2D>, static_cast<uint32_t>(SignDirection::Count)> signs_;

	// parameters
	// アニメーションに使用するパラメータは全て共通
	Vector2 emitPos_;           // 発生座標
	StateTimer animationTimer_; // タイマー
	LerpValue<Vector2> size_;   // サイズ
	LerpValue<float> move_;     // 開始地点からの移動量

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// update
	void UpdateAnimation();

	// helper
	DirectionInfo GetDirectionInfo(SignDirection direction) const;
};