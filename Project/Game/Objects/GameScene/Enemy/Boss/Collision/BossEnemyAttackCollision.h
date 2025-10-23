#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Collision/Collider.h>
#include <Game/Objects/GameScene/Enemy/Boss/Structures/BossEnemyStructures.h>

//============================================================================
//	BossEnemyAttackCollision class
//	ボスの攻撃の当たり判定
//============================================================================
class BossEnemyAttackCollision :
	public Collider {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyAttackCollision() = default;
	~BossEnemyAttackCollision() = default;

	// 初期化
	void Init();

	// 衝突判定更新
	void Update(const Transform3D& transform);

	// エディター
	void ImGui();

	// json
	void ApplyJson(const Json& data);
	void SaveJson(Json& data);

	// 衝突コールバック関数
	void OnCollisionEnter(const CollisionBody* collisionBody) override;

	//--------- accessor -----------------------------------------------------

	// 状態設定
	void SetEnterState(BossEnemyState state);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 判定区間
	struct TimeWindow {

		float on;  // 衝突開始時間(判定入り)
		float off; // 衝突終了時間(判定を消す)
	};

	struct AttackParameter {

		Vector3 centerOffset; // player座標からのオフセット
		Vector3 size;         // サイズ

		std::vector<TimeWindow> windows;
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	BossEnemyState currentState_;

	CollisionBody* weaponBody_;

	std::unordered_map<BossEnemyState, AttackParameter> table_; // 状態毎の衝突

	float currentTimer_; // 現在の経過時間、全部共通

	// editor
	BossEnemyState editingState_;

	//--------- functions ----------------------------------------------------

	// helper
	BossEnemyState GetBossEnemyStateFromName(const std::string& name);
	void EditWindowParameter(const std::string& label, std::vector<TimeWindow>& windows);
};