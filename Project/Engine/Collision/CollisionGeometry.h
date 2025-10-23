#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/MathUtils.h>

// c++
#include <variant>

//============================================================================
//	CollisionShape namespace
//	基本形状(Sphere/AABB/OBB)のパラメータとjson入出力、複合Variant型を提供する。
//============================================================================
namespace CollisionShape {

	//----------------------------------------------------------------------------
	//	Sphere
	//	中心と半径で定義される球。デフォルト生成とjson入出力をサポート。
	//----------------------------------------------------------------------------
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

	//----------------------------------------------------------------------------
	//	AABB
	//	中心と半径ベクトル(extent)で定義される軸平行境界ボックス。min/max取得ヘルパを持つ。
	//----------------------------------------------------------------------------
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

	//----------------------------------------------------------------------------
	//	OBB
	//	中心/サイズ/回転で定義される有向境界ボックス。オイラーとクォータニオン双方を保持。
	//----------------------------------------------------------------------------
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

	// Sphere/AABB/OBBの何れかを保持する共用型
	using Shapes = std::variant<Sphere, AABB, OBB>;
};

// 判定用の形状種別
enum class ShapeType {

	Type_Sphere,
	Type_AABB,
	Type_OBB
};

//============================================================================
//	Collision namespace
//	形状間の交差判定(3D/2D)のユーティリティ関数群を提供する。
//============================================================================
namespace Collision {

	//========================================================================
	//	3D
	//========================================================================

	//--------- sphere -------------------------------------------------------

	// 2球の中心距離と半径和で交差を判定する
	bool SphereToSphere(const CollisionShape::Sphere& sphereA, const CollisionShape::Sphere& sphereB);

	// 球中心をAABBにクランプして最近接点から交差を判定する
	bool SphereToAABB(const CollisionShape::Sphere& sphere, const CollisionShape::AABB& aabb);

	// OBBの軸に投影した最近接点から交差を判定する
	bool SphereToOBB(const CollisionShape::Sphere& sphere, const CollisionShape::OBB& obb);

	//--------- aabb ---------------------------------------------------------

	// 軸毎の重なり有無で交差を判定する
	bool AABBToAABB(const CollisionShape::AABB& aabbA, const CollisionShape::AABB& aabbB);

	// AABBをOBBに変換してOBB同士の判定へ委譲する
	bool AABBToOBB(const CollisionShape::AABB& aabb, const CollisionShape::OBB& obbB);

	//--------- obb ----------------------------------------------------------

	// 分離軸の投影長と中心距離の比較で交差を判定する
	bool OBBToOBB(const CollisionShape::OBB& obbA, const CollisionShape::OBB& obbB);

	//--------- reverse order helpers ---------------------------------------

	// 引数順の違いを吸収して既存実装へ委譲する
	bool AABBToSphere(const CollisionShape::AABB& aabb, const CollisionShape::Sphere& sphere);
	bool OBBToSphere(const CollisionShape::OBB& obb, const CollisionShape::Sphere& sphere);
	bool OBBToAABB(const CollisionShape::OBB& obb, const CollisionShape::AABB& aabb);

	//========================================================================
	//	2D
	//========================================================================

	// 画面内の矩形とマウス座標のヒットを判定する
	bool RectToMouse(const Vector2& center, const Vector2& size, const Vector2& anchor);
}