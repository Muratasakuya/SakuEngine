#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject2D.h>
#include <Engine/Utility/Timer/StateTimer.h>

//============================================================================
//	FadeSprite class
//	画面にフェードイン・フェードアウトをかけるスプライト
//============================================================================
class FadeSprite :
	public GameObject2D {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FadeSprite() = default;
	~FadeSprite() = default;

	void DerivedInit() override;

	void Update();

	void DerivedImGui() override;

	void Start();
	void Reset();

	//--------- accessor -----------------------------------------------------

	bool IsFinished() const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 状態
	enum class State {

		None,
		Begin,
		Wait,
		End,
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;
	bool disableTransition_;

	// タイマー管理
	StateTimer beginTimer_;
	StateTimer waitTimer_;
	StateTimer endTimer_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// update
};