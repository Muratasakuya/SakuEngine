#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/SubPlayer/State/Interface/SubPlayerIState.h>

//============================================================================
//	SubPlayerInactiveState class
//============================================================================
class SubPlayerInactiveState :
	public SubPlayerIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SubPlayerInactiveState() = default;
	~SubPlayerInactiveState() = default;

	// 状態遷移時
	void Enter() override;

	// 更新処理
	void Update() override;

	// 状態終了時
	void Exit() override;

	// エディター
	void ImGui() override;

	// json
	void ApplyJson(const Json& data) override;
	void SaveJson(Json& data) override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------



	//--------- functions ----------------------------------------------------

};