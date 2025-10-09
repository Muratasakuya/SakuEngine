#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/AssetStructure.h>
#include <Engine/Object/Data/Transform.h>
#include <Engine/Core/Graphics/GPUObject/VertexBuffer.h>
#include <Engine/Core/Graphics/GPUObject/IndexBuffer.h>
#include <Engine/Core/Graphics/DxLib/DxStructures.h>
#include <Engine/Core/Graphics/PostProcess/PostProcessType.h>

// directX
#include <Externals/DirectXTex/DirectXTex.h>
// c++
#include <string>
// front
class Asset;

//============================================================================
//	enum class
//============================================================================

// 描画を行う場所
enum class SpriteLayer {

	PreModel, // modelの前に描画する
	PostModel // modelの後に描画する
};

// スプライトの描画順インデックス
enum class SpriteLayerIndex :
	uint16_t {

	None = 0,              // 一番手前の表示(初期化順で決まる)
	SceneTransition = 128, // シーン遷移処理
};

//============================================================================
//	Sprite class
//============================================================================
class Sprite {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Sprite() = default;
	Sprite(ID3D12Device* device, Asset* asset,
		const std::string& textureName, Transform2D& transform);
	~Sprite() = default;

	void UpdateVertex(const Transform2D& transform);

	void ImGui(float itemSize);

	//--------- accessor -----------------------------------------------------

	void SetTextureName(const std::string& textureName) { textureName_ = textureName; }
	void SetAlphaTextureName(const std::string& textureName) { alphaTextureName_ = textureName; }

	void SetLayer(SpriteLayer layer) { layer_ = layer; }
	void SetLayerIndex(SpriteLayerIndex layerIndex, uint16_t subLayerIndex) { layerIndex_ = static_cast<uint16_t>(layerIndex) + subLayerIndex; }
	void SetPostProcessEnable(bool enable) { postProcessEnable_ = enable; }
	void SetPostEffectMask(uint8_t mask) { postEffectMask_ = mask; }
	void SetBlendMode(BlendMode blendMode) { blendMode_ = blendMode; }

	static uint32_t GetIndexNum() { return kIndexNum_; }

	SpriteLayer GetLayer() const { return layer_; }
	uint16_t GetLayerIndex() const { return static_cast<uint16_t>(layerIndex_); }
	bool GetPostProcessEnable() const { return postProcessEnable_; }
	uint8_t GetPostEffectMask() const { return postEffectMask_; }
	bool UseAlphaTexture() const { return alphaTextureName_.has_value(); }
	BlendMode GetBlendMode() const { return blendMode_; }

	const VertexBuffer<SpriteVertexData>& GetVertexBuffer() const { return vertexBuffer_; }
	const IndexBuffer& GetIndexBuffer() const { return indexBuffer_; }

	const D3D12_GPU_DESCRIPTOR_HANDLE& GetTextureGPUHandle() const;
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetAlphaTextureGPUHandle() const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	static constexpr const uint32_t kVertexNum_ = 4;
	static constexpr const uint32_t kIndexNum_ = 6;

	Asset* asset_;

	std::string textureName_;
	std::string preTextureName_;
	std::optional<std::string> alphaTextureName_;
	DirectX::TexMetadata metadata_;

	// 描画順制御
	SpriteLayer layer_;
	uint16_t layerIndex_ = static_cast<uint16_t>(SpriteLayerIndex::None);

	// ポストエフェクト
	bool postProcessEnable_;
	uint8_t postEffectMask_ = static_cast<uint8_t>(PostEffectBit::None);

	// 頂点情報
	std::vector<SpriteVertexData> vertexData_;
	BlendMode blendMode_ = BlendMode::kBlendModeNormal;

	// buffer
	VertexBuffer<SpriteVertexData> vertexBuffer_;
	IndexBuffer indexBuffer_;

	//--------- functions ----------------------------------------------------

	// init
	void InitBuffer(ID3D12Device* device);
	void SetMetaDataTextureSize(Transform2D& transform);
};