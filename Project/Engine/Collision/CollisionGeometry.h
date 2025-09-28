#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/MathUtils.h>

// c++
#include <variant>

//============================================================================
//	CollisionShape namespace
//============================================================================
namespace CollisionShape {

	struct Sphere {

		Vector3 center;
		float radius;

		static Sphere Default() {
			Sphere sphere = {
				.center = Vector3::AnyInit(0.0f),
				.radius = 1.0f
			};
			return sphere;
		};

		void ToJson(Json& data);
		void FromJson(const Json& data);
	};

	struct AABB {

		Vector3 center;
		Vector3 extent;

		Vector3 GetMin() const { return center - extent; }
		Vector3 GetMax() const { return center + extent; }

		static AABB Default() {
			AABB aabb = {

				.center = Vector3::AnyInit(0.0f),
				.extent = Vector3::AnyInit(1.0f),
			};
			return aabb;
		};

		void ToJson(Json& data);
		void FromJson(const Json& data);
	};

	struct OBB {

		Vector3 center;
		Vector3 size;
		Vector3 eulerRotate;
		Quaternion rotate;

		static OBB Default() {
			OBB obb = {
				.center = Vector3::AnyInit(0.0f),
				.size = Vector3::AnyInit(1.0f),
				.rotate = Quaternion::IdentityQuaternion()
			};
			return obb;
		};

		void ToJson(Json& data);
		void FromJson(const Json& data);
	};

	using Shapes = std::variant<Sphere, AABB, OBB>;
};

enum class ShapeType {

	Type_Sphere,
	Type_AABB,
	Type_OBB
};

//============================================================================
//	Collision namespace
//============================================================================
namespace Collision {

	//============================================================================
	//	3D
	//============================================================================

	// sphere
	bool SphereToSphere(const CollisionShape::Sphere& sphereA, const CollisionShape::Sphere& sphereB);
	bool SphereToAABB(const CollisionShape::Sphere& sphere, const CollisionShape::AABB& aabb);
	bool SphereToOBB(const CollisionShape::Sphere& sphere, const CollisionShape::OBB& obb);

	// aabb
	bool AABBToAABB(const CollisionShape::AABB& aabbA, const CollisionShape::AABB& aabbB);
	bool AABBToOBB(const CollisionShape::AABB& aabb, const CollisionShape::OBB& obbB);

	// oobb
	bool OBBToOBB(const CollisionShape::OBB& obbA, const CollisionShape::OBB& obbB);

	// 順番が異なる場合
	bool AABBToSphere(const CollisionShape::AABB& aabb, const CollisionShape::Sphere& sphere);
	bool OBBToSphere(const CollisionShape::OBB& obb, const CollisionShape::Sphere& sphere);
	bool OBBToAABB(const CollisionShape::OBB& obb, const CollisionShape::AABB& aabb);

	//============================================================================
	//	2D
	//============================================================================

	bool RectToMouse(const Vector2& center, const Vector2& size,
		const Vector2& anchor);
}