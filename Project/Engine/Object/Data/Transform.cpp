#include "Transform.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Utility/JsonAdapter.h>
#include <Engine/Editor/ImGuiObjectEditor.h>

// imgui
#include <imgui.h>

//============================================================================
//	Transform classMethods
//============================================================================

//============================================================================
// 3D
//============================================================================

void BaseTransform::Init() {

	scale = Vector3::AnyInit(1.0f);
	rotation.Init();
	translation.Init();

	eulerRotate.Init();
	prevScale = Vector3::AnyInit(1.0f);
}

void BaseTransform::UpdateMatrix() {

	// 値に変更がなければ更新しない
	bool selfUnchanged =
		(scale == prevScale &&
			rotation == prevRotation &&
			translation == prevTranslation);

	// 親と自分の値が変わっていなければ更新しない
	if (selfUnchanged && (!parent || !parent->IsDirty())) {

		isDirty_ = false;
		return;
	}
	// どちらかに変更があれば更新
	isDirty_ = true;

	// 行列を更新
	matrix.Update(parent, scale, rotation, translation);

	// 値を保存
	prevScale = scale;
	prevRotation = rotation;
	prevTranslation = translation;
}

void BaseTransform::ImGui(float itemSize) {

	ImGui::PushItemWidth(itemSize);
	if (ImGui::Button("Reset")) {

		scale = Vector3::AnyInit(1.0f);
		rotation.Init();
		translation.Init();

		eulerRotate.Init();
	}
	ImGui::Text(std::format("isDirty: {}", isDirty_).c_str());

	ImGui::DragFloat3("translation", &translation.x, 0.01f);
	if (ImGui::DragFloat3("rotation", &eulerRotate.x, 0.01f)) {

		rotation = Quaternion::EulerToQuaternion(eulerRotate);
	}
	ImGui::Text("quaternion(%4.3f, %4.3f, %4.3f, %4.3f)",
		rotation.x, rotation.y, rotation.z, rotation.w);
	ImGui::DragFloat3("scale", &scale.x, 0.01f);

	ImGui::SeparatorText("World Matrix");
	if (ImGui::BeginTable("WorldMatrix", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {

		const Matrix4x4& world = matrix.world;
		for (int row = 0; row < 4; ++row) {

			ImGui::TableNextRow();
			for (int col = 0; col < 4; ++col) {

				ImGui::TableSetColumnIndex(col);
				ImGui::Text("%.3f", world.m[row][col]);
			}
		}
		ImGui::EndTable();
	}
	// 親がいる場合
	if (parent) {

		ImGui::SeparatorText("Parent World Matrix");

		ImGui::Text(std::format("isDirty {}", parent->isDirty_).c_str());
		if (ImGui::BeginTable("Parent WorldMatrix", 4,
			ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {

			const Matrix4x4& world = parent->matrix.world;
			for (int row = 0; row < 4; ++row) {

				ImGui::TableNextRow();
				for (int col = 0; col < 4; ++col) {

					ImGui::TableSetColumnIndex(col);
					ImGui::Text("%.3f", world.m[row][col]);
				}
			}
			ImGui::EndTable();
		}
	}

	ImGui::PopItemWidth();
}

void BaseTransform::ToJson(Json& data) {

	data["scale"] = scale.ToJson();
	data["rotation"] = rotation.ToJson();
	data["translation"] = translation.ToJson();
}

void BaseTransform::FromJson(const Json& data) {

	scale = JsonAdapter::ToObject<Vector3>(data["scale"]);
	rotation = JsonAdapter::ToObject<Quaternion>(data["rotation"]);
	translation = JsonAdapter::ToObject<Vector3>(data["translation"]);
}

Vector3 BaseTransform::GetWorldPos() const {

	Vector3 worldPos{};
	worldPos.x = matrix.world.m[3][0];
	worldPos.y = matrix.world.m[3][1];
	worldPos.z = matrix.world.m[3][2];

	return worldPos;
}

Vector3 BaseTransform::GetForward() const {
	return Vector3(matrix.world.m[2][0], matrix.world.m[2][1], matrix.world.m[2][2]).Normalize();
}

Vector3 BaseTransform::GetBack() const {
	return Vector3(-GetForward().x, -GetForward().y, -GetForward().z);
}

Vector3 BaseTransform::GetRight() const {
	return Vector3(matrix.world.m[0][0], matrix.world.m[0][1], matrix.world.m[0][2]).Normalize();
}

Vector3 BaseTransform::GetLeft() const {
	return Vector3(-GetRight().x, -GetRight().y, -GetRight().z);
}

Vector3 BaseTransform::GetUp() const {
	return Vector3(matrix.world.m[1][0], matrix.world.m[1][1], matrix.world.m[1][2]).Normalize();
}

Vector3 BaseTransform::GetDown() const {
	return Vector3(-GetUp().x, -GetUp().y, -GetUp().z);
}

//============================================================================
// 2D
//============================================================================

void Transform2D::Init(ID3D12Device* device) {

	translation = Vector2::AnyInit(0.0f);
	rotation = 0.0f;

	size = Vector2::AnyInit(0.0f);
	// 中心で設定
	anchorPoint = Vector2::AnyInit(0.5f);

	// 左上設定
	textureLeftTop = Vector2::AnyInit(0.0f);
	textureSize = Vector2::AnyInit(0.0f);

	// buffer初期化
	buffer_.CreateBuffer(device);
}

void Transform2D::UpdateMatrix() {

	Vector3 scale = Vector3(size.x, size.y, 1.0f);
	Vector3 rotate = Vector3(0.0f, 0.0f, rotation);
	Vector3 translate = Vector3(translation.x, translation.y, 0.0f);

	matrix = Matrix4x4::MakeAffineMatrix(scale, rotate, translate);

	if (parent) {
		matrix = parent->matrix * matrix;
	}

	// buffer転送
	buffer_.TransferData(matrix);
}

void Transform2D::ImGui(float itemSize) {

	ImGui::SeparatorText("Config");

	if (ImGui::Button("Set CenterPos", ImVec2(itemSize, 40.0f))) {

		SetCenterPos();
	}

	if (ImGui::Button("Set CenterAnchor", ImVec2(itemSize, 40.0f))) {

		anchorPoint = Vector2::AnyInit(0.5f);
	}

	if (ImGui::Button("Set LeftAnchor", ImVec2(itemSize, 40.0f))) {

		anchorPoint = Vector2::AnyInit(0.0f);
	}

	if (ImGui::Button("Set RightAnchor", ImVec2(itemSize, 40.0f))) {

		anchorPoint = Vector2::AnyInit(1.0f);
	}

	ImGui::SeparatorText("Parameter");

	ImGui::PushItemWidth(itemSize);
	ImGui::DragFloat2("translation", &translation.x, 1.0f);
	ImGui::SliderAngle("rotation", &rotation);

	ImGui::DragFloat2("size", &size.x, 1.0f);
	ImGui::DragFloat2("anchorPoint", &anchorPoint.x, 0.01f, 0.0f, 1.0f);

	ImGui::DragFloat2("textureLeftTop", &textureLeftTop.x, 1.0f);
	ImGui::DragFloat2("textureSize", &textureSize.x, 1.0f);
	ImGui::PopItemWidth();
}

void Transform2D::SetCenterPos() {

	translation.x = Config::kWindowWidthf / 2.0f;
	translation.y = Config::kWindowHeightf / 2.0f;
}