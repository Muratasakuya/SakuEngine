#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>
#include <Game/Objects/GameScene/Environment/Collision/FieldWallCollision.h>

//============================================================================
//	FieldBoundary class
//	設定されたオブジェクトをAABBで囲み、範囲外に出た場合に押し戻す処理を行う
//============================================================================
class FieldBoundary :
	public IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FieldBoundary() :IGameEditor("FieldWallCollisionCollection") {}
	~FieldBoundary() = default;

	// 初期化
	void Init();

	// 制御処理
	void ControlTargetMove();

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// 押し戻し対象の設定
	void SetPushBackTarget(Player* player, BossEnemy* bossEnemy);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Player* player_;
	BossEnemy* bossEnemy_;

	// AABBの押し戻し領域
	std::vector<std::unique_ptr<FieldWallCollision>> collisions_;
	// 座標移動制限
	float moveClampLength_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// update
	void UpdateAllCollisionBody();
};