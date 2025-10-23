#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Collision/Collider.h>
#include <Engine/Object/Data/Transform.h>

//============================================================================
//	BossEnemyBladeCollision class
//	敵が出す刃の当たり判定
//============================================================================
class BossEnemyBladeCollision :
	public Collider {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyBladeCollision() = default;
	~BossEnemyBladeCollision() = default;
	
	// 初期化、typeNameは刃の種類のファイル
	void Init(const std::string& typeName);

	// 更新
	void Update();

	// エディター
	void ImGui();

	//--------- accessor -----------------------------------------------------

	// 発生させて動かす
	void EmitEffect(const Vector3& emitPos, const Vector3& velocity);

	const BaseTransform& GetTransform() const { return transform_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 名前
	std::string typeName_;
	std::string fileName_;

	// collisionに渡す値
	Transform3D transform_;

	bool isEmit_;      // 発生させたかどうか
	Vector3 velocity_; // 移動速度(方向も含む)

	// parameters
	Vector3 scale_;   // 大きさ
	float lifeTimer_; // 生存時間の経過時間
	float lifeTime_;  // 生存時間

	float outsideTranslationY_; // 場外Y座標

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// update
	void UpdateMove();
	void UpdateLifeTime();
};