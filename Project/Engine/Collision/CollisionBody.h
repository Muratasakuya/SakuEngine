#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Collision/CollisionGeometry.h>

// c++
#include <cstdint>
#include <optional>
#include <functional>

//============================================================================
//	ColliderType
//	衝突グループ/マスクを表すビットフラグ。自己タイプと相手許可タイプの組で判定に用いる
//============================================================================

enum class ColliderType {

	Type_None = 0, // ビットが立っていない状態
	Type_Test = 1 << 0,
	Type_Player = 1 << 1,
	Type_PlayerWeapon = 1 << 2,
	Type_BossEnemy = 1 << 3,
	Type_BossWeapon = 1 << 4,
	Type_BossBlade = 1 << 5,
	Type_CrossMarkWall = 1 << 6,
	Type_FieldWall = 1 << 7,
	Type_Event = 1 << 8,
};

// operator(ビット演算子の定義)
inline ColliderType operator|(ColliderType lhs, ColliderType rhs) {
	using T = std::underlying_type_t<ColliderType>;
	return static_cast<ColliderType>(static_cast<T>(lhs) | static_cast<T>(rhs));
}
inline ColliderType& operator|=(ColliderType& lhs, ColliderType rhs) {
	lhs = lhs | rhs;
	return lhs;
}
inline ColliderType operator&(ColliderType lhs, ColliderType rhs) {
	using T = std::underlying_type_t<ColliderType>;
	return static_cast<ColliderType>(static_cast<T>(lhs) & static_cast<T>(rhs));
}
inline ColliderType& operator&=(ColliderType& lhs, ColliderType rhs) {
	lhs = lhs & rhs;
	return lhs;
}

//============================================================================
//	CollisionBody  class
//	単一の衝突形状とタイプ/ターゲット設定、当たりコールバックを保持する最小単位。
//============================================================================
class CollisionBody {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	//--------- functions ----------------------------------------------------

	CollisionBody() = default;
	~CollisionBody() = default;

	// コールバックを安全に発火させる(Enter/Stay/Exit)
	void TriggerOnCollisionEnter(CollisionBody* collider);
	void TriggerOnCollisionStay(CollisionBody* collider);
	void TriggerOnCollisionExit(CollisionBody* collider);

	// 当たり時に呼ばれるコールバック関数を設定する
	void SetOnCollisionEnter(std::function<void(CollisionBody*)> onCollisionEnter) { onEnter_ = onCollisionEnter; }
	void SetOnCollisionStay(std::function<void(CollisionBody*)> onCollisionEnter) { onStay_ = onCollisionEnter; }
	void SetOnCollisionExit(std::function<void(CollisionBody*)> onCollisionEnter) { onExit_ = onCollisionEnter; }

	// 形状タイプに応じて内部形状を更新する(不一致ならASSERT)
	void UpdateSphere(const CollisionShape::Sphere& sphere);
	void UpdateAABB(const CollisionShape::AABB& aabb);
	void UpdateOBB(const CollisionShape::OBB& obb);

	//--------- accessor -----------------------------------------------------

	// 形状/タイプ/ターゲットを設定・取得する

	void SetShape(const CollisionShape::Shapes& shape) { shape_ = shape; }

	void SetType(ColliderType type) { type_ = type; }
	void SetTargetType(ColliderType target) { targetType_ = target; }

	ColliderType GetType() const { return type_; }
	ColliderType GetTargetType() const { return targetType_; }

	const CollisionShape::Shapes& GetShape() const { return shape_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- using --------------------------------------------------------

	// 当たりコールバックのシグネチャ
	using CollisionCallback = std::function<void(CollisionBody*)>;

	//--------- variables ----------------------------------------------------

	// コールバック関数
	CollisionCallback onEnter_ = nullptr;
	CollisionCallback onStay_ = nullptr;
	CollisionCallback onExit_ = nullptr;

	ColliderType type_;       // 自身の衝突タイプ
	ColliderType targetType_; // 衝突相手のタイプ

	CollisionShape::Shapes shape_; // 衝突判定を行う形状
};