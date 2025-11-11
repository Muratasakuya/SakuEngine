#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Animation/SimpleAnimation.h>

// front
class BossEnemy;

//============================================================================
//	BossEnemyStartAnimation class
//	ボスの登場アニメーション
//============================================================================
class BossEnemyStartAnimation {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyStartAnimation() = default;
	~BossEnemyStartAnimation() = default;

	// 初期化
	void Init();

	// 更新
	void Update(BossEnemy& bossEnemy);

	// 開始呼び出し
	void Start(BossEnemy& bossEnemy);

	// エディター
	void ImGui(BossEnemy& bossEnemy);

	//--------- accessor -----------------------------------------------------

private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 処理が開始されたか
	bool isStarted_;

	// 開始呼び出しからの待機時間
	StateTimer delayTimer_;
	bool isWaited_;

	// 座標アニメーション
	SimpleAnimation<Vector3> posAnimation_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};