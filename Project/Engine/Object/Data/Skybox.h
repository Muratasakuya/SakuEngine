#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>
#include <Engine/Core/Graphics/GPUObject/VertexBuffer.h>
#include <Engine/Core/Graphics/GPUObject/IndexBuffer.h>
#include <Engine/MathLib/MathUtils.h>

// data
#include <Engine/Object/Data/Transform.h>
#include <Engine/Object/Data/Material.h>
#include <Engine/Object/Data/ObjectTag.h>

//============================================================================
//	Skybox class
//	スカイボックスの初期化から更新、描画までを行う
//============================================================================
class Skybox {
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// マテリアル構造体
	struct SkyboxMaterial {

		Color color;
		uint32_t textureIndex;
		float pad0[3];
		Matrix4x4 uvTransform;
		uint32_t postProcessMask;
	};
public:
	//========================================================================
	//	public Methods
	//========================================================================

	Skybox() = default;
	~Skybox() = default;

	// 初期化
	void Create(ID3D12Device* device, uint32_t textureIndex, uint32_t objectIndex);

	// 更新
	void Update();

	// エディター
	void ImGui(float itemSize);

	//--------- accessor -----------------------------------------------------

	uint32_t GetIndexCount() const { return indexCount_; }
	uint32_t GetTextureIndex() const { return material_.textureIndex; }

	// buffers
	const VertexBuffer<Vector4>& GetVertexBuffer() const { return vertexBuffer_; }
	const IndexBuffer& GetIndexBuffer() const { return indexBuffer_; }

	const DxConstBuffer<Matrix4x4>& GetMatrixBuffer() const { return matrixBuffer_; }
	const DxConstBuffer<SkyboxMaterial>& GetMaterialBuffer() const { return materialBuffer_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ObjectTag* tag_;

	// 頂点buffer
	VertexBuffer<Vector4> vertexBuffer_;
	// 頂点インデックスbuffer
	IndexBuffer indexBuffer_;
	uint32_t indexCount_;

	BaseTransform transform_;
	SkyboxMaterial material_;

	// uv
	UVTransform uvTransform_;
	UVTransform prevUVTransform_;

	// cBuffer
	DxConstBuffer<Matrix4x4> matrixBuffer_;
	DxConstBuffer<SkyboxMaterial> materialBuffer_;

	//--------- functions ----------------------------------------------------

	// バッファ作成
	void CreateVertexBuffer(ID3D12Device* device);
	void CreateCBuffer(ID3D12Device* device, uint32_t textureIndex);

	// update
	void UpdateUVTransform();
};