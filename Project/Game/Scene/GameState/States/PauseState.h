#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Scene/GameState/Interface/IGameSceneState.h>

//============================================================================
//	PauseState class
//	ポーズ中の状態
//============================================================================
class PauseState :
	public IGameSceneState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PauseState(GameContext* context) :IGameSceneState(context) {}
	~PauseState() = default;

	void Init(SceneView* sceneView) override;

	void Update(SceneManager* sceneManager) override;
	void NonActiveUpdate(SceneManager* sceneManager) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------



	//--------- functions ----------------------------------------------------

};