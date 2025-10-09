#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector3.h>
#include <Engine/MathLib/Vector4.h>
#include <Engine/MathLib/Quaternion.h>
#include <Engine/MathLib/Matrix4x4.h>

// c++
#include <cstdint>
#include <string>
// front
class BaseTransform;

//============================================================================
//	CBufferStructures
//============================================================================

struct TransformationMatrix {

	Matrix4x4 world;
	Matrix4x4 worldInverseTranspose;

	void Update(const BaseTransform* parent, const Vector3& scale,
		const Quaternion& rotation, const Vector3& translation,
		const std::optional<Matrix4x4>& billboardMatrix = std::nullopt);
};

struct MaterialForGPU {

	Color color;

	uint32_t textureIndex;
	uint32_t normalMapTextureIndex;
	int32_t enableNormalMap;
	int32_t enableDithering;

	float emissiveIntensity;
	Vector3 emissionColor;

	Matrix4x4 uvTransform;

	uint32_t postProcessMask;
};

struct LightingForGPU {

	int32_t enableLighting;
	int32_t enableHalfLambert;
	int32_t enableBlinnPhongReflection;
	int32_t enableImageBasedLighting;
	int32_t castShadow;

	float phongRefShininess;
	Vector3 specularColor;

	float shadowRate;
	float environmentCoefficient;
};

struct SpriteMaterialForGPU {

	Matrix4x4 uvTransform;
	Color color;
	Vector3 emissionColor;
	int32_t useVertexColor;
	int32_t useAlphaColor;
	float emissiveIntensity;
	float alphaReference;
	uint32_t postProcessMask;

	void Init();
	void ImGui();
};