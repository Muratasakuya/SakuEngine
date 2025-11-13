#include "DefaultDistortionUpdater.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

//============================================================================
//	DefaultDistortionUpdater classMethods
//============================================================================

void DefaultDistortionUpdater::Init() {

	// デフォルト
	processTextureName_ = "distortion_1";

	// json適応
	ApplyJson();
}

void DefaultDistortionUpdater::Update() {

	// UVの更新
	bufferData_.uvTransform = Matrix4x4::MakeAffineMatrix(
		Vector3(scale_.x, scale_.y, 1.0f),
		Vector3(0.0f, 0.0f, rotationZ_),
		Vector3(translation_.x, translation_.y, 0.0f));
}

void DefaultDistortionUpdater::ImGui() {

	SaveButton();
	ImGui::Separator();

	// テクスチャ選択
	// 表示サイズ
	const float imageSize = 88.0f;
	ImGuiHelper::ImageButtonWithLabel("texture", processTextureName_,
		(ImTextureID)asset_->GetGPUHandle(processTextureName_).ptr, { imageSize, imageSize });
	std::string dragTextureName = ImGuiHelper::DragDropPayloadString(PendingType::Texture);
	if (!dragTextureName.empty()) {

		// textureを設定
		processTextureName_ = dragTextureName;
	}

	ImGui::SeparatorText("Param");

	ImGui::DragFloat("bias", &bufferData_.bias, 0.001f);
	ImGui::DragFloat("strength", &bufferData_.strength, 0.001f);

	ImGui::SeparatorText("UV");

	ImGui::DragFloat2("translation", &translation_.x, 0.001f);
	ImGui::DragFloat("rotationZ", &rotationZ_, 0.001f);
	ImGui::DragFloat2("scale", &scale_.x, 0.001f);
}

void DefaultDistortionUpdater::ApplyJson() {

	Json data;
	if (!LoadFile(data)) {
		return;
	}

	processTextureName_ = data["distortionTextureName"].get<std::string>();

	bufferData_.bias = data["bias"].get<float>();
	bufferData_.strength = data["strength"].get<float>();

	translation_ = Vector2::FromJson(data["translation"]);
	scale_ = Vector2::FromJson(data["scale"]);
	rotationZ_ = data["rotationZ"].get<float>();
}

void DefaultDistortionUpdater::SaveJson() {

	Json data;

	data["distortionTextureName"] = processTextureName_;

	data["bias"] = bufferData_.bias;
	data["strength"] = bufferData_.strength;

	data["translation"] = translation_.ToJson();
	data["scale"] = scale_.ToJson();
	data["rotationZ"] = rotationZ_;

	SaveFile(data);
}