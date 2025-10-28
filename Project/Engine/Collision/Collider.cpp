#include "Collider.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/Transform.h>
#include <Engine/Collision/CollisionManager.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/Algorithm.h>

// imgui
#include <imgui.h>

//============================================================================
//	Collider classMethods
//============================================================================

Collider::~Collider() {

	if (bodies_.empty()) {
		return;
	}

	for (const auto& body : bodies_) {

		RemoveCollider(body);
	}
}

void Collider::UpdateAllBodies(const Transform3D& transform) {

	if (bodies_.empty()) {
		return;
	}

	for (size_t index = 0; index < bodies_.size(); ++index) {

		const auto& body = bodies_[index];
		const auto& offset = bodyOffsets_[index];

		std::visit([&](const auto& shape) {
			using T = std::decay_t<decltype(shape)>;
			if constexpr (std::is_same_v<T, CollisionShape::Sphere>) {

				UpdateSphereBody(body, transform, std::get<CollisionShape::Sphere>(offset));
			} else if constexpr (std::is_same_v<T, CollisionShape::AABB>) {

				UpdateAABBBody(body, transform, std::get<CollisionShape::AABB>(offset));
			} else if constexpr (std::is_same_v<T, CollisionShape::OBB>) {

				UpdateOBBBody(body, transform, std::get<CollisionShape::OBB>(offset));
			}
			}, offset);
	}
}

int Collider::ToIndexType(ColliderType type) {

	if (type == ColliderType::Type_None) {
		return 0;
	}
	return std::countr_zero(static_cast<std::make_unsigned_t<std::underlying_type_t<ColliderType>>>(type)) + 1;
}

void Collider::UpdateSphereBody(CollisionBody* body, const Transform3D& transform, const CollisionShape::Sphere& offset) {

	// 子か親かで座標を変える
	Vector3 bodyTranslation = isChild_ ? transform.GetWorldPos() : transform.translation;
	Vector3 center = bodyTranslation + offset.center;

	body->UpdateSphere(CollisionShape::Sphere(center, offset.radius));
}

void Collider::UpdateAABBBody(CollisionBody* body, const Transform3D& transform, const CollisionShape::AABB& offset) {

	// 子か親かで座標を変える
	Vector3 bodyTranslation = isChild_ ? transform.GetWorldPos() : transform.translation;
	Vector3 center = bodyTranslation + offset.center;
	Vector3 extent = transform.scale * offset.extent;

	body->UpdateAABB(CollisionShape::AABB(center, extent));
}

void Collider::UpdateOBBBody(CollisionBody* body, const Transform3D& transform, const CollisionShape::OBB& offset) {

	// 子か親かで座標を変える
	Vector3 bodyTranslation = isChild_ ? transform.GetWorldPos() : transform.translation;
	Vector3 center = bodyTranslation + offset.center;
	Vector3 size = transform.scale + offset.size;

	// 子か親かで回転を変える
	Quaternion bodyRotation = isChild_ ? Quaternion::FromRotationMatrix(transform.matrix.world) : transform.rotation;
	Quaternion rotation = (bodyRotation * offset.rotate).Normalize();

	body->UpdateOBB(CollisionShape::OBB(center, size, Vector3::AnyInit(0.0f), rotation));
}

CollisionBody* Collider::AddCollider(const CollisionShape::Shapes& shape, bool autoAddOffset) {

	CollisionBody* collider = nullptr;
	collider = CollisionManager::GetInstance()->AddCollisionBody(shape);

	// 関数呼び出し
	currentState_ = State::None;
	collider->SetOnCollisionEnter([this](CollisionBody* otherCollider) {

		OnCollisionEnter(otherCollider);
		currentState_ = State::Enter;
		});
	collider->SetOnCollisionStay([this](CollisionBody* otherCollider) {

		OnCollisionStay(otherCollider);
		currentState_ = State::Stay;
		});
	collider->SetOnCollisionExit([this](CollisionBody* otherCollider) {

		OnCollisionExit(otherCollider);
		currentState_ = State::Exit;
		});

	// オフセットを追加
	if (autoAddOffset) {

		bodies_.emplace_back(collider);
		bodyOffsets_.emplace_back(shape);
	}

	return collider;
}

void Collider::RemoveCollider(CollisionBody* collisionBody) {

	CollisionManager::GetInstance()->RemoveCollisionBody(collisionBody);
}

void Collider::ImGui(float itemWidth) {

	if (bodyOffsets_.empty()) {
		return;
	}

	ImGui::PushItemWidth(itemWidth);

	ImGui::Text("currentState: %s", EnumAdapter<State>::ToString(currentState_));

	for (uint32_t index = 0; index < bodyOffsets_.size(); ++index) {

		auto& offset = bodyOffsets_[index];

		std::visit([&](const auto& shape) {
			using T = std::decay_t<decltype(shape)>;
			if constexpr (std::is_same_v<T, CollisionShape::Sphere>) {

				EditSphereBody(index, std::get<CollisionShape::Sphere>(offset));
			} else if constexpr (std::is_same_v<T, CollisionShape::AABB>) {

				EditAABBBody(index, std::get<CollisionShape::AABB>(offset));
			} else if constexpr (std::is_same_v<T, CollisionShape::OBB>) {

				EditOBBBody(index, std::get<CollisionShape::OBB>(offset));
			}
			}, offset);
		ImGui::Separator();
	}

	ImGui::PopItemWidth();
}

void Collider::EditSphereBody(uint32_t index, CollisionShape::Sphere& offset) {

	ImGui::Text("body: Sphere");
	ImGui::Text("bodyIndex: %d", index);

	ImGui::DragFloat3(("center" + std::to_string(index)).c_str(), &offset.center.x, 0.01f);
	ImGui::DragFloat(("radius" + std::to_string(index)).c_str(), &offset.radius, 0.01f);
}

void Collider::EditAABBBody(uint32_t index, CollisionShape::AABB& offset) {

	ImGui::Text("body: AABB");
	ImGui::Text("bodyIndex: %d", index);

	ImGui::DragFloat3(("center" + std::to_string(index)).c_str(), &offset.center.x, 0.01f);
	ImGui::DragFloat3(("extent" + std::to_string(index)).c_str(), &offset.extent.x, 0.01f);
}

void Collider::EditOBBBody(uint32_t index, CollisionShape::OBB& offset) {

	ImGui::Text("body: OBB");
	ImGui::Text("bodyIndex: %d", index);

	ImGui::DragFloat3(("center" + std::to_string(index)).c_str(), &offset.center.x, 0.01f);

	if (ImGui::DragFloat3(("eulerRotate" + std::to_string(index)).c_str(), &offset.eulerRotate.x, 0.01f)) {

		offset.rotate = Quaternion::EulerToQuaternion(offset.eulerRotate);
	}
	ImGui::Text("quaternion(%4.3f, %4.3f, %4.3f, %4.3f)",
		offset.rotate.x, offset.rotate.y, offset.rotate.z, offset.rotate.w);

	ImGui::DragFloat3(("size" + std::to_string(index)).c_str(), &offset.size.x, 0.01f);
}

void Collider::ApplyBodyOffset(const Json& data) {

	for (uint32_t index = 0; index < bodyOffsets_.size(); ++index) {

		std::string label = Algorithm::GetIndexLabel("CollisionBody", index);
		auto& offset = bodyOffsets_[index];

		std::visit([&](const auto& shape) {
			using T = std::decay_t<decltype(shape)>;
			if constexpr (std::is_same_v<T, CollisionShape::Sphere>) {

				std::get<CollisionShape::Sphere>(offset).FromJson(data[label]);
			} else if constexpr (std::is_same_v<T, CollisionShape::AABB>) {

				std::get<CollisionShape::AABB>(offset).FromJson(data[label]);
			} else if constexpr (std::is_same_v<T, CollisionShape::OBB>) {

				std::get<CollisionShape::OBB>(offset).FromJson(data[label]);
			}
			}, offset);
	}
}

void Collider::SaveBodyOffset(Json& data) {

	for (uint32_t index = 0; index < bodyOffsets_.size(); ++index) {

		std::string label = Algorithm::GetIndexLabel("CollisionBody", index);
		auto& offset = bodyOffsets_[index];

		std::visit([&](const auto& shape) {
			using T = std::decay_t<decltype(shape)>;
			if constexpr (std::is_same_v<T, CollisionShape::Sphere>) {

				std::get<CollisionShape::Sphere>(offset).ToJson(data[label]);
			} else if constexpr (std::is_same_v<T, CollisionShape::AABB>) {

				std::get<CollisionShape::AABB>(offset).ToJson(data[label]);
			} else if constexpr (std::is_same_v<T, CollisionShape::OBB>) {

				std::get<CollisionShape::OBB>(offset).ToJson(data[label]);
			}
			}, offset);
	}
}

void Collider::BuildBodies(const Json& data) {

	// 配列か単一のデータかどうか
	auto makeRange = [](const Json& json) -> std::vector<Json> {
		if (json.is_array()) {
			return { json.begin(), json.end() };
		} else { return { json }; }};

	// colliderのjsonデータを回して設定する
	uint32_t index = 0;
	for (const Json& colliderData : makeRange(data)) {

		//  形状文字列を小文字統一
		std::string shapeString = colliderData.value("shape", "AABB");
		std::transform(shapeString.begin(), shapeString.end(), shapeString.begin(),
			[](unsigned char c) { return static_cast<char>(::tolower(c)); });

		// 形状設定
		if (!SetShapeParamFromJson(shapeString, colliderData)) {
			continue;
		}

		// 衝突ボディ追加
		CollisionBody* body = bodies_.emplace_back(AddCollider(bodyOffsets_[index]));
		// タイプ設定
		SetTypeFromJson(*body, colliderData);

		++index;
	}
}

void Collider::SetSphereRadius(float radius, std::optional<uint32_t> index) {

	// 指定インデックスの球形状の半径を設定する
	if (index.has_value()) {

		auto& offset = bodyOffsets_[index.value()];
		std::visit([&](const auto& shape) {
			using T = std::decay_t<decltype(shape)>;
			if constexpr (std::is_same_v<T, CollisionShape::Sphere>) {

				std::get<CollisionShape::Sphere>(offset).radius = radius;
			}
			}, offset);
	}
	// 全ての球形状の半径を設定する
	else {
		for (uint32_t i = 0; i < bodyOffsets_.size(); ++i) {

			auto& offset = bodyOffsets_[i];
			std::visit([&](const auto& shape) {
				using T = std::decay_t<decltype(shape)>;
				if constexpr (std::is_same_v<T, CollisionShape::Sphere>) {

					std::get<CollisionShape::Sphere>(offset).radius = radius;
				}
				}, offset);
		}
	}
}

bool Collider::SetShapeParamFromJson(const std::string& shapeName, const Json& data) {

	if (shapeName == "sphere") {

		auto sphere = CollisionShape::Sphere::Default();
		if (data.contains("center")) {

			sphere.center = JsonAdapter::ToObject<Vector3>(data["center"]);
		}
		if (data.contains("radius")) {

			sphere.radius = JsonAdapter::GetValue<float>(data, "radius");
		}
		bodyOffsets_.emplace_back(sphere);
		return true;
	} else if (shapeName == "aabb") {

		auto aabb = CollisionShape::AABB::Default();
		if (data.contains("center")) {

			aabb.center = JsonAdapter::ToObject<Vector3>(data["center"]);
		}
		if (data.contains("extent")) {

			aabb.extent = JsonAdapter::ToObject<Vector3>(data["extent"]);
		}
		bodyOffsets_.emplace_back(aabb);
		return true;
	} else if (shapeName == "obb") {

		auto obb = CollisionShape::OBB::Default();
		if (data.contains("center")) {

			obb.center = JsonAdapter::ToObject<Vector3>(data["center"]);
		}
		if (data.contains("size")) {

			obb.size = JsonAdapter::ToObject<Vector3>(data["size"]);
			obb.size.x *= 2.0f;
		}
		if (data.contains("rotate")) {

			obb.rotate = JsonAdapter::ToObject<Quaternion>(data["rotate"]);
		}
		bodyOffsets_.emplace_back(obb);
		return true;
	}

	// 作成できなかった
	return false;
}

void Collider::SetTypeFromJson(CollisionBody& body, const Json& data) {

	// 自身のタイプ
	if (data.contains("selfType")) {

		std::string selfTypeName = data["selfType"].get<std::string>();
		body.SetType(ToColliderType(selfTypeName));
	}

	// 衝突相手
	if (data.contains("targetTypes")) {

		ColliderType mask = ColliderType::Type_None;
		for (const auto& targetName : data["targetTypes"]) {

			mask |= ToColliderType(targetName.get<std::string>());
		}
		body.SetTargetType(mask);
	}
}

ColliderType Collider::ToColliderType(const std::string& name) const {

	if (name == "None") return ColliderType::Type_None;
	if (name == "Test") return ColliderType::Type_Test;
	if (name == "Player") return ColliderType::Type_Player;
	if (name == "CrossMarkWall") return ColliderType::Type_CrossMarkWall;

	return ColliderType::Type_None;
}