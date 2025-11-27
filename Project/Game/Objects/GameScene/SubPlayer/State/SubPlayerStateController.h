#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Objects/GameScene/SubPlayer/State/Interface/SubPlayerIState.h>
#include <Game/Objects/GameScene/SubPlayer/Structure/SubPlayerStructure.h>

//============================================================================
//	SubPlayerStateController class
//	サブプレイヤーの状態の管理
//============================================================================
class SubPlayerStateController {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SubPlayerStateController() = default;
	~SubPlayerStateController() = default;

	// 初期化
	void Init();

	// 更新
	void Update();

	// エディター
	void ImGui();

	//--------- accessor -----------------------------------------------------

	void SetBossEnemy(const BossEnemy* bossEnemy);

	// 状態をリクエスト
	void SetRequestState(SubPlayerState state) { requestState_ = state; }

	// 各パーツを状態に設定
	void SetParts(GameObject3D* body, GameObject3D* rightHand, GameObject3D* leftHand);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 現在の状態
	SubPlayerState current_;
	// リクエストされた状態
	std::optional<SubPlayerState> requestState_;

	// 各状態を処理する
	std::unordered_map<SubPlayerState, std::unique_ptr<SubPlayerIState>> states_;

	// エディター
	SubPlayerState editState_;
	SubPlayerState editRequestState_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// 状態切り替え
	void ChangeState(bool isForce);
	// 状態が終了したかチェックする
	void CheckCanExit();
};