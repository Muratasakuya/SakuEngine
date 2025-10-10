#include "Sprite.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

// imgui
#include <imgui.h>

//============================================================================
//	Sprite classMethods
//============================================================================

Sprite::Sprite(ID3D12Device* device, Asset* asset,
	const std::string& textureName, Transform2D& transform) {

	asset_ = nullptr;
	asset_ = asset;

	textureName_ = textureName;
	preTextureName_ = textureName;
	metadata_ = asset_->GetMetaData(textureName_);

	layer_ = SpriteLayer::PostModel;

	// buffer作成
	InitBuffer(device);

	// textureSizeにtransformを合わせる
	SetMetaDataTextureSize(transform);
}

void Sprite::UpdateVertex(const Transform2D& transform) {

	float left = 0.0f - transform.anchorPoint.x;
	float right = 1.0f - transform.anchorPoint.x;
	float top = 0.0f - transform.anchorPoint.y;
	float bottom = 1.0f - transform.anchorPoint.y;

	// textureに変更があったときのみ
	if (preTextureName_ != textureName_) {

		// metaData更新
		metadata_ = asset_->GetMetaData(textureName_);
		preTextureName_ = textureName_;
	}

	// 横
	float texLeft = transform.textureLeftTop.x / static_cast<float>(metadata_.width);
	float texRight = (transform.textureLeftTop.x + transform.textureSize.x) / static_cast<float>(metadata_.width);
	// 縦
	float texTop = transform.textureLeftTop.y / static_cast<float>(metadata_.height);
	float texBottom = (transform.textureLeftTop.y + transform.textureSize.y) / static_cast<float>(metadata_.height);

	// vertexデータの更新
	// 左下
	vertexData_[0].pos = Vector2(left, bottom) + transform.vertexOffset_[0];
	vertexData_[0].texcoord = { texLeft,texBottom };
	vertexData_[0].color = Color::White();
	// 左上
	vertexData_[1].pos = Vector2(left, top) + transform.vertexOffset_[1];
	vertexData_[1].texcoord = { texLeft,texTop };
	vertexData_[1].color = Color::White();
	// 右下
	vertexData_[2].pos = Vector2(right, bottom) + transform.vertexOffset_[2];
	vertexData_[2].texcoord = { texRight,texBottom };
	vertexData_[2].color = Color::White();
	// 右上
	vertexData_[3].pos = Vector2(right, top) + transform.vertexOffset_[3];
	vertexData_[3].texcoord = { texRight,texTop };
	vertexData_[3].color = Color::White();

	// GPUデータ転送
	vertexBuffer_.TransferData(vertexData_);
}

void Sprite::InitBuffer(ID3D12Device* device) {

	// buffer作成
	vertexBuffer_.CreateBuffer(device, kVertexNum_);
	indexBuffer_.CreateBuffer(device, kIndexNum_);

	// vertexデータの初期化
	vertexData_.resize(kVertexNum_);

	// indexデータの初期化
	std::vector<uint32_t> indexData(kIndexNum_);

	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;

	// GPUデータ転送
	indexBuffer_.TransferData(indexData);
}

void Sprite::SetMetaDataTextureSize(Transform2D& transform) {

	float left = 0.0f - transform.anchorPoint.x;
	float right = 1.0f - transform.anchorPoint.x;
	float top = 0.0f - transform.anchorPoint.y;
	float bottom = 1.0f - transform.anchorPoint.y;

	// textureMetadataの取得
	const DirectX::TexMetadata& metadata = asset_->GetMetaData(textureName_);

	// textureSizeの設定
	transform.textureSize = { static_cast<float>(metadata.width) ,static_cast<float>(metadata.height) };
	transform.size = transform.textureSize;

	// 横
	float texLeft = transform.textureLeftTop.x / static_cast<float>(metadata.width);
	float texRight = (transform.textureLeftTop.x + transform.textureSize.x) / static_cast<float>(metadata.width);
	// 縦
	float texTop = transform.textureLeftTop.y / static_cast<float>(metadata.height);
	float texBottom = (transform.textureLeftTop.y + transform.textureSize.y) / static_cast<float>(metadata.height);

	// vertexデータの初期化
	// 左下
	vertexData_[0].pos = { left,bottom };
	vertexData_[0].texcoord = { texLeft,texBottom };
	// 左上
	vertexData_[1].pos = { left,top };
	vertexData_[1].texcoord = { texLeft,texTop };
	// 右下
	vertexData_[2].pos = { right,bottom };
	vertexData_[2].texcoord = { texRight,texBottom };
	// 右上
	vertexData_[3].pos = { right,top };
	vertexData_[3].texcoord = { texRight,texTop };
}


void Sprite::ImGui(float itemSize) {

	ImGui::PushItemWidth(itemSize);
	
	EnumAdapter<SpriteLayer>::Combo("SpriteLayer", &layer_);

	ImGui::PopItemWidth();
}

const D3D12_GPU_DESCRIPTOR_HANDLE& Sprite::GetTextureGPUHandle() const {

	return asset_->GetGPUHandle(textureName_);
}

const D3D12_GPU_DESCRIPTOR_HANDLE& Sprite::GetAlphaTextureGPUHandle() const {

	if (!alphaTextureName_.has_value()) {
		assert(false && "alpha texture name is not set");
	}

	return asset_->GetGPUHandle(alphaTextureName_.value());
}
