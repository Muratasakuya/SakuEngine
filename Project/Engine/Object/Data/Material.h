#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/GPUObject/CBufferStructures.h>
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>
#include <Engine/Utility/Animation/SimpleAnimation.h>
#include <Engine/MathLib/MathUtils.h>

// front
class Asset;

//============================================================================
//	UVTransform
//============================================================================

struct UVTransform {

	Vector3 scale;
	Vector3 rotate;
	Vector3 translation;

	// operator
	bool operator==(const UVTransform& other) const {

		return scale == other.scale &&
			rotate == other.rotate &&
			translation == other.translation;
	}
};

//============================================================================
//	Material class
//============================================================================
class Material {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Material() = default;
	~Material() = default;

	void Init(Asset* asset);

	void UpdateUVTransform();

	void ImGui(float itemSize);

	void ToJson(Json& data);
	void FromJson(const Json& data);

	//--------- accessor -----------------------------------------------------

	void SetTextureName(const std::string& textureName);

	//--------- variables ----------------------------------------------------

	Color color; // 色

	// テクスチャインデックス
	uint32_t textureIndex;
	uint32_t normalMapTextureIndex;

	// 法線マップ
	int32_t enableNormalMap;

	// ディザ抜き
	int32_t enableDithering;

	// ライティング
	int32_t enableLighting;
	int32_t enableHalfLambert;
	int32_t enableBlinnPhongReflection;
	
	float phongRefShininess;
	Vector3 specularColor;

	// 影
	int32_t castShadow;
	float shadowRate;

	// 環境マップ
	int32_t enableImageBasedLighting;
	float environmentCoefficient;

	// 発光度
	float emissiveIntensity;
	Vector3 emissionColor;

	// uv
	Matrix4x4 uvMatrix;
	UVTransform uvTransform;

	// 適応するポストエフェクトのビット
	uint32_t postProcessMask;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Asset* asset_;

	UVTransform prevUVTransform_;

	//--------- functions ----------------------------------------------------

	// init
	void InitParameter();
};

//============================================================================
//	SpriteMaterial class
//============================================================================
class SpriteMaterial {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SpriteMaterial() = default;
	~SpriteMaterial() = default;

	void Init(ID3D12Device* device);

	void UpdateUVTransform();

	void ImGui(float itemSize);

	void ToJson(Json& data);
	void FromJson(const Json& data);

	//--------- variables ----------------------------------------------------

	SpriteMaterialForGPU material;

	UVTransform uvTransform;

	//--------- accessor -----------------------------------------------------

	const DxConstBuffer<SpriteMaterialForGPU>& GetBuffer() const { return buffer_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	UVTransform prevUVTransform_;

	// buffer
	DxConstBuffer<SpriteMaterialForGPU> buffer_;
};