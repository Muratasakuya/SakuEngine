#include "CollisionGeometry.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Engine/Utility/JsonAdapter.h>

//============================================================================
//	CollisionShape Methods
//============================================================================

void CollisionShape::Sphere::ToJson(Json& data) {

	data["Sphere"]["center"] = JsonAdapter::FromObject<Vector3>(center);
	data["Sphere"]["radius"] = radius;
}

void CollisionShape::Sphere::FromJson(const Json& data) {

	center = JsonAdapter::ToObject<Vector3>(data["Sphere"]["center"]);
	radius = data["Sphere"]["radius"];
}

void CollisionShape::AABB::ToJson(Json& data) {

	data["AABB"]["center"] = JsonAdapter::FromObject<Vector3>(center);
	data["AABB"]["extent"] = JsonAdapter::FromObject<Vector3>(extent);
}

void CollisionShape::AABB::FromJson(const Json& data) {

	center = JsonAdapter::ToObject<Vector3>(data["AABB"]["center"]);
	extent = JsonAdapter::ToObject<Vector3>(data["AABB"]["extent"]);
}

void CollisionShape::OBB::ToJson(Json& data) {

	data["OBB"]["center"] = JsonAdapter::FromObject<Vector3>(center);
	data["OBB"]["size"] = JsonAdapter::FromObject<Vector3>(size);
	data["OBB"]["eulerRotate"] = JsonAdapter::FromObject<Vector3>(eulerRotate);
	data["OBB"]["rotate"] = JsonAdapter::FromObject<Quaternion>(rotate);
}

void CollisionShape::OBB::FromJson(const Json& data) {

	center = JsonAdapter::ToObject<Vector3>(data["OBB"]["center"]);
	size = JsonAdapter::ToObject<Vector3>(data["OBB"]["size"]);
	eulerRotate = JsonAdapter::ToObject<Vector3>(data["OBB"]["eulerRotate"]);
	rotate = JsonAdapter::ToObject<Quaternion>(data["OBB"]["rotate"]);
}

//============================================================================
//	Collision Methods
//============================================================================

bool Collision::SphereToSphere(const CollisionShape::Sphere& sphereA,
	const CollisionShape::Sphere& sphereB) {

	float distance = Vector3(sphereA.center - sphereB.center).Length();
	if (distance <= sphereA.radius + sphereB.radius) {
		return true;
	}

	return false;
}

bool Collision::SphereToAABB(const CollisionShape::Sphere& sphere, const CollisionShape::AABB& aabb) {

	const Vector3 min = aabb.GetMin();
	const Vector3 max = aabb.GetMax();

	// 球の中心をAABB内にクランプしてAABB上の最近接点を求める
	Vector3 closestPoint(
		std::clamp(sphere.center.x, min.x, max.x),
		std::clamp(sphere.center.y, min.y, max.y),
		std::clamp(sphere.center.z, min.z, max.z));

	// 最近接点と球中心との距離の二乗を比較
	Vector3 diff = closestPoint - sphere.center;
	float distanceSquared = Vector3::Dot(diff, diff);

	return distanceSquared <= (sphere.radius * sphere.radius);
}

bool Collision::SphereToOBB(const CollisionShape::Sphere& sphere,
	const CollisionShape::OBB& obb) {

	Matrix4x4 rotateMatrix = Quaternion::MakeRotateMatrix(obb.rotate);

	Vector3 orientations[3];
	orientations[0] = Vector3::Transform(Vector3(1.0f, 0.0f, 0.0f), rotateMatrix);
	orientations[1] = Vector3::Transform(Vector3(0.0f, 1.0f, 0.0f), rotateMatrix);
	orientations[2] = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f), rotateMatrix);

	Vector3 localSphereCenter = sphere.center - obb.center;
	Vector3 closestPoint = obb.center;

	for (int i = 0; i < 3; ++i) {

		float distance = Vector3::Dot(localSphereCenter, orientations[i]);
		float halfSize = (i == 0) ? obb.size.x : (i == 1) ? obb.size.y : obb.size.z;

		if (distance > halfSize) {
			distance = halfSize;
		} else if (distance < -halfSize) {
			distance = -halfSize;
		}

		closestPoint += distance * orientations[i];
	}

	Vector3 diff = closestPoint - sphere.center;
	float distanceSquared = Vector3::Dot(diff, diff);

	return distanceSquared <= (sphere.radius * sphere.radius);
}

bool Collision::AABBToOBB(const CollisionShape::AABB& aabb, const CollisionShape::OBB& obbB) {

	CollisionShape::OBB obbA;
	obbA.center = aabb.center;
	obbA.size = aabb.extent;
	obbA.rotate = Quaternion::IdentityQuaternion();

	return OBBToOBB(obbA, obbB);
}

bool Collision::AABBToAABB(const CollisionShape::AABB& aabbA, const CollisionShape::AABB& aabbB) {

	Vector3 minA = aabbA.GetMin();
	Vector3 maxA = aabbA.GetMax();
	Vector3 minB = aabbB.GetMin();
	Vector3 maxB = aabbB.GetMax();

	// 各軸で重なっているか確認
	bool overlapX = (minA.x <= maxB.x) && (maxA.x >= minB.x);
	bool overlapY = (minA.y <= maxB.y) && (maxA.y >= minB.y);
	bool overlapZ = (minA.z <= maxB.z) && (maxA.z >= minB.z);

	return overlapX && overlapY && overlapZ;
}

bool Collision::OBBToOBB(const CollisionShape::OBB& obbA, const CollisionShape::OBB& obbB) {

	auto CalculateProjection =
		[](const CollisionShape::OBB& obb, const Vector3& axis, const Vector3* axes) -> float {
		return std::abs(obb.size.x * Vector3::Dot(axes[0], axis)) +
			std::abs(obb.size.y * Vector3::Dot(axes[1], axis)) +
			std::abs(obb.size.z * Vector3::Dot(axes[2], axis));
		};

	auto GetOBBAxes = [](const CollisionShape::OBB& obb) -> std::array<Vector3, 3> {
		Matrix4x4 rotationMatrix = Quaternion::MakeRotateMatrix(obb.rotate);
		return {
			Vector3::Transform(Vector3(1.0f, 0.0f, 0.0f), rotationMatrix),
			Vector3::Transform(Vector3(0.0f, 1.0f, 0.0f), rotationMatrix),
			Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f), rotationMatrix)
		};
		};

	auto obbAxesA = GetOBBAxes(obbA);
	auto obbAxesB = GetOBBAxes(obbB);

	std::vector<Vector3> axes;
	axes.insert(axes.end(), obbAxesA.begin(), obbAxesA.end());
	axes.insert(axes.end(), obbAxesB.begin(), obbAxesB.end());
	for (const auto& axisA : obbAxesA) {
		for (const auto& axisB : obbAxesB) {
			axes.push_back(Vector3::Cross(axisA, axisB));
		}
	}

	for (const auto& axis : axes) {
		if (axis.Length() < std::numeric_limits<float>::epsilon()) {
			continue;
		}

		Vector3 normalizedAxis = axis.Normalize();

		float obbAProjection = CalculateProjection(obbA, normalizedAxis, obbAxesA.data());
		float obbBProjection = CalculateProjection(obbB, normalizedAxis, obbAxesB.data());
		float distance = std::abs(Vector3::Dot(obbA.center - obbB.center, normalizedAxis));

		if (distance > obbAProjection + obbBProjection) {
			return false;
		}
	}

	return true;
}

bool Collision::AABBToSphere(const CollisionShape::AABB& aabb, const CollisionShape::Sphere& sphere) {

	return SphereToAABB(sphere, aabb);
}

bool Collision::OBBToSphere(const CollisionShape::OBB& obb, const CollisionShape::Sphere& sphere) {

	return SphereToOBB(sphere, obb);
}

bool Collision::OBBToAABB(const CollisionShape::OBB& obb, const CollisionShape::AABB& aabb) {

	return AABBToOBB(aabb, obb);
}

bool Collision::RectToMouse(const Vector2& center,
	const Vector2& size, const Vector2& anchor) {

	Input* input = Input::GetInstance();
	if (!input->IsMouseOnView(InputViewArea::Game)) {
		return false;
	}
	
	// マウス位置
	Vector2 mousePos = input->GetMousePosInView(InputViewArea::Game).value();

	// 矩形左上
	Vector2 topLeft = center - Vector2(size.x * anchor.x, size.y * anchor.y);
	Vector2 bottomRight = topLeft + size;
	if (mousePos.x >= topLeft.x && mousePos.x <= bottomRight.x &&
		mousePos.y >= topLeft.y && mousePos.y <= bottomRight.y) {

		return true;
	}
	return false;
}