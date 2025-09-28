#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/StateTimer.h>
#include <Engine/Object/Base/GameObject2D.h>
#include <Game/Objects/Base/ITransition.h>

//============================================================================
//	FadeTransition class
//============================================================================
class FadeTransition :
	public ITransition {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FadeTransition() = default;
	~FadeTransition() = default;

	void Init() override;

	void Update() override;

	void BeginUpdate() override;

	void LoadUpdate() override;
	void LoadEndUpdate() override;

	void EndUpdate() override;

	void ImGui() override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// フェード用
	std::unique_ptr<GameObject2D> fadeSprite_;
	// ロード中用
	std::unique_ptr<GameObject2D> loadSprite_;

	// タイマー管理
	StateTimer beginTimer_;
	StateTimer waitTimer_;
	StateTimer endTimer_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};