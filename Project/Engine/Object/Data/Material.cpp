#include "Material.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

// imgui
#include <imgui.h>

//============================================================================
//	Material classMethods
//============================================================================

void Material::InitParameter() {

	color = Color::White();
	enableNormalMap = false;
	enableDithering = false;
	enableLighting = true;
	enableHalfLambert = true;
	enableBlinnPhongReflection = false;
	enableImageBasedLighting = false;
	castShadow = true;
	environmentCoefficient = 0.25f;
	shadowRate = 0.25f;
	phongRefShininess = 1.0f;
	specularColor = Vector3(1.0f, 1.0f, 1.0f);
	emissiveIntensity = 0.0f;
	emissionColor = Vector3(1.0f, 1.0f, 1.0f);
	uvMatrix = Matrix4x4::MakeIdentity4x4();
}

void Material::Init(Asset* asset) {

	InitParameter();
	uvTransform.scale = Vector3::AnyInit(1.0f);
	prevUVTransform_.scale = Vector3::AnyInit(1.0f);

	asset_ = nullptr;
	asset_ = asset;
}

void Material::UpdateUVTransform() {

	// 値に変更がなければ更新しない
	if (uvTransform == prevUVTransform_) {
		return;
	}

	// uvの更新
	uvMatrix = Matrix4x4::MakeAffineMatrix(uvTransform.scale, uvTransform.rotate, uvTransform.translation);

	// 値を保存
	prevUVTransform_ = uvTransform;
}

void Material::ImGui(float itemSize) {

	// 色
	ImGui::SeparatorText("Color");

	ImGui::PushItemWidth(itemSize);
	ImGui::ColorEdit4("color", &color.r);
	ImGui::Text("R:%4.3f G:%4.3f B:%4.3f A:%4.3f",
		color.r, color.g,
		color.b, color.a);

	// 発行色
	ImGui::ColorEdit3("emissionColor", &emissionColor.x);
	ImGui::Text("R:%4.3f G:%4.3f B:%4.3f",
		emissionColor.x, emissionColor.y,
		emissionColor.z);
	ImGui::DragFloat("emissiveIntensity", &emissiveIntensity, 0.01f);

	// UV
	ImGui::SeparatorText("UV");

	// transform
	ImGui::DragFloat2("uvTranslate", &uvTransform.translation.x, 0.1f);
	ImGui::SliderAngle("uvRotate", &uvTransform.rotate.z);
	ImGui::DragFloat2("uvScale", &uvTransform.scale.x, 0.1f);

	// Lighting
	ImGui::SeparatorText("Lighting");

	ImGui::SliderInt("enableLighting", &enableLighting, 0, 1);
	ImGui::SliderInt("blinnPhongReflection", &enableBlinnPhongReflection, 0, 1);

	if (enableBlinnPhongReflection) {

		ImGui::ColorEdit3("specularColor", &specularColor.x);
		ImGui::DragFloat("phongRefShininess", &phongRefShininess, 0.01f);
	}

	ImGui::SliderInt("enableImageBasedLighting", &enableImageBasedLighting, 0, 1);
	if (enableImageBasedLighting) {

		ImGui::DragFloat("environmentCoefficient", &environmentCoefficient, 0.001f, 0.0f, 4.0f);
	}

	ImGui::SliderInt("castShadow", &castShadow, 0, 1);
	ImGui::DragFloat("shadowRate", &shadowRate, 0.01f, 0.0f, 8.0f);

	ImGui::SliderInt("enableDithering", &enableDithering, 0, 1);

	ImGui::SeparatorText("PostProcess");

	ImGuiHelper::EditPostProcessMask(postProcessMask);

	ImGui::PopItemWidth();
}

void Material::ToJson(Json& data) {

	// Material
	// color
	data["color"] = color.ToJson();
	data["emissionColor"] = emissionColor.ToJson();
	data["emissiveIntensity"] = emissiveIntensity;
	data["enableDithering"] = enableDithering;
	// lighting
	data["enableLighting"] = enableLighting;
	data["enableHalfLambert"] = enableHalfLambert;
	data["enableBlinnPhongReflection"] = enableBlinnPhongReflection;
	data["enableImageBasedLighting"] = enableImageBasedLighting;
	data["castShadow"] = castShadow;
	data["phongRefShininess"] = phongRefShininess;
	data["specularColor"] = specularColor.ToJson();
	data["shadowRate"] = shadowRate;
	data["environmentCoefficient"] = environmentCoefficient;

	// UV
	data["uvScale"] = uvTransform.scale.ToJson();
	data["uvRotate"] = uvTransform.rotate.ToJson();
	data["uvTranslate"] = uvTransform.translation.ToJson();
}

void Material::FromJson(const Json& data) {

	// Material
	// color
	color = JsonAdapter::ToObject<Color>(data["color"]);
	emissionColor = JsonAdapter::ToObject<Vector3>(data["emissionColor"]);
	emissiveIntensity = JsonAdapter::GetValue<float>(data, "emissiveIntensity");
	enableDithering = JsonAdapter::GetValue<int32_t>(data, "enableDithering");
	// lighting
	enableLighting = JsonAdapter::GetValue<int32_t>(data, "enableLighting");
	enableHalfLambert = JsonAdapter::GetValue<int32_t>(data, "enableHalfLambert");
	enableBlinnPhongReflection = JsonAdapter::GetValue<int32_t>(data, "enableBlinnPhongReflection");
	enableImageBasedLighting = JsonAdapter::GetValue<int32_t>(data, "enableImageBasedLighting");
	castShadow = JsonAdapter::GetValue<int32_t>(data, "castShadow");
	phongRefShininess = JsonAdapter::GetValue<float>(data, "phongRefShininess");
	specularColor = JsonAdapter::ToObject<Vector3>(data["specularColor"]);
	shadowRate = JsonAdapter::GetValue<float>(data, "shadowRate");
	environmentCoefficient = JsonAdapter::GetValue<float>(data, "environmentCoefficient");

	// UV
	uvTransform.scale = JsonAdapter::ToObject<Vector3>(data["uvScale"]);
	uvTransform.rotate = JsonAdapter::ToObject<Vector3>(data["uvRotate"]);
	uvTransform.translation = JsonAdapter::ToObject<Vector3>(data["uvTranslate"]);
}

void Material::SetTextureName(const std::string& textureName) {

	textureIndex = asset_->GetTextureGPUIndex(textureName);
}

//============================================================================
//	SpriteMaterial classMethods
//============================================================================

void SpriteMaterial::Init(ID3D12Device* device) {

	material.Init();
	uvTransform.scale = Vector3::AnyInit(1.0f);
	prevUVTransform_.scale = Vector3::AnyInit(1.0f);

	// buffer初期化
	buffer_.CreateBuffer(device);
}

void SpriteMaterial::UpdateUVTransform() {

	// 値に変更がなければ更新しない
	if (uvTransform == prevUVTransform_) {

		// buffer転送
		buffer_.TransferData(material);
		return;
	}

	// uvの更新
	material.uvTransform = Matrix4x4::MakeAffineMatrix(
		uvTransform.scale, uvTransform.rotate, uvTransform.translation);

	// 値を保存
	prevUVTransform_ = uvTransform;
}

void SpriteMaterial::ImGui(float itemSize) {

	// 色
	ImGui::SeparatorText("Color");

	ImGui::PushItemWidth(itemSize);
	ImGui::ColorEdit4("color", &material.color.r);
	ImGui::Text("R:%4.3f G:%4.3f B:%4.3f A:%4.3f",
		material.color.r, material.color.g,
		material.color.b, material.color.a);

	ImGui::SliderInt("useAlphaColor", &material.useAlphaColor, 0, 1);
	if (material.useAlphaColor) {

		ImGui::DragFloat("alphaReference", &material.alphaReference, 0.001f);
	}

	// 発行色
	ImGui::ColorEdit3("emissionColor", &material.emissionColor.x);
	ImGui::Text("R:%4.3f G:%4.3f B:%4.3f",
		material.emissionColor.x, material.emissionColor.y,
		material.emissionColor.z);
	ImGui::DragFloat("emissiveIntensity", &material.emissiveIntensity, 0.01f);

	// UV
	ImGui::SeparatorText("UV");

	// transform
	ImGui::DragFloat2("uvTranslate", &uvTransform.translation.x, 0.1f);
	ImGui::SliderAngle("uvRotate", &uvTransform.rotate.z);
	ImGui::DragFloat2("uvScale", &uvTransform.scale.x, 0.1f);

	ImGui::SeparatorText("PostProcess");

	ImGuiHelper::EditPostProcessMask(material.postProcessMask);

	ImGui::PopItemWidth();
}

void SpriteMaterial::ToJson(Json& data) {

	// Material
	// color
	data["color"] = material.color.ToJson();
	data["emissionColor"] = material.emissionColor.ToJson();
	data["emissiveIntensity"] = material.emissiveIntensity;
	data["useVertexColor"] = material.useVertexColor;

	// UV
	data["uvScale"] = uvTransform.scale.ToJson();
	data["uvRotate"] = uvTransform.rotate.ToJson();
	data["uvTranslate"] = uvTransform.translation.ToJson();
}

void SpriteMaterial::FromJson(const Json& data) {

	// Material
	// color
	material.color = JsonAdapter::ToObject<Color>(data["color"]);
	material.emissionColor = JsonAdapter::ToObject<Vector3>(data["emissionColor"]);
	material.emissiveIntensity = data["emissiveIntensity"];
	material.useVertexColor = data["useVertexColor"];

	// UV
	uvTransform.scale = JsonAdapter::ToObject<Vector3>(data["uvScale"]);
	uvTransform.rotate = JsonAdapter::ToObject<Vector3>(data["uvRotate"]);
	uvTransform.translation = JsonAdapter::ToObject<Vector3>(data["uvTranslate"]);
}