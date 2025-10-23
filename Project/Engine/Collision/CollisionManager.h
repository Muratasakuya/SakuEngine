#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Collision/CollisionBody.h>
#include <Engine/Collision/CollisionGeometry.h>

// c++
#include <list>
#include <utility>
#include <unordered_set>

//============================================================================
//	CollisionManager class
//	衝突ボディ群の生成/破棄/更新と、Enter/Stay/Exitのディスパッチ、デバッグ描画を行う。
//============================================================================
class CollisionManager {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	// 新規CollisionBodyを生成して内部リストに追加する
	CollisionBody* AddCollisionBody(const CollisionShape::Shapes& shape);

	// 指定ボディを内部リストから除去し、関連する事前衝突ペアも破棄する
	void RemoveCollisionBody(CollisionBody* collider);

	// 全ての衝突情報をクリアする(ボディ/ペア)
	void ClearAllCollision();

	// 全ボディの組合せを走査し、衝突状態に応じてEnter/Stay/Exitを発火する
	void Update();

	//--------- accessor -----------------------------------------------------

	// シングルトン取得/破棄
	static CollisionManager* GetInstance();
	static void Finalize();
private:
	//========================================================================
	//	private Methods
	//========================================================================
	
	// ハッシュセット用のペアハッシュ
	struct pair_hash {
		template <class T1, class T2>
		std::size_t operator () (const std::pair<T1, T2>& pair) const {
			return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
		}
	};

	//--------- variables ----------------------------------------------------

	static CollisionManager* instance_;

	std::list<CollisionBody*> colliders_;
	std::unordered_set<std::pair<CollisionBody*, CollisionBody*>, pair_hash> preCollisions_;

	//--------- functions ----------------------------------------------------

	// 2つのボディ形状に応じたディスパッチテーブルで交差を判定する
	bool IsColliding(CollisionBody* colliderA, CollisionBody* colliderB);

	// デバッグ用に各コライダー形状を描画する
	void DrawCollider();

	CollisionManager() = default;
	~CollisionManager() = default;
	CollisionManager(const CollisionManager&) = delete;
	CollisionManager& operator=(const CollisionManager&) = delete;
};