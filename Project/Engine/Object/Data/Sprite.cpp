#include "Sprite.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Asset/Asset.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

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

	float left = (0.0f - transform.anchorPoint.x) * transform.size.x;
	float right = (1.0f - transform.anchorPoint.x) * transform.size.x;
	float top = (0.0f - transform.anchorPoint.y) * transform.size.y;
	float bottom = (1.0f - transform.anchorPoint.y) * transform.size.y;

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

	// textureMetadataの取得
	const DirectX::TexMetadata& metadata = asset_->GetMetaData(textureName_);

	// textureSizeの設定
	transform.textureSize = { static_cast<float>(metadata.width) ,static_cast<float>(metadata.height) };
	transform.size = transform.textureSize;

	// アンカー基準のピクセル矩形
	float left = (0.0f - transform.anchorPoint.x) * transform.size.x;
	float right = (1.0f - transform.anchorPoint.x) * transform.size.x;
	float top = (0.0f - transform.anchorPoint.y) * transform.size.y;
	float bottom = (1.0f - transform.anchorPoint.y) * transform.size.y;

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

	ImGui::Separator();

	// テクスチャ選択
	// 表示サイズ
	const float imageSize = 88.0f;
	ImGuiHelper::ImageButtonWithLabel("texture", textureName_,
		(ImTextureID)asset_->GetGPUHandle(textureName_).ptr, { imageSize, imageSize });
	std::string dragTextureName = ImGuiHelper::DragDropPayloadString(PendingType::Texture);
	if (!dragTextureName.empty()) {

		// textureを設定
		textureName_ = dragTextureName;
	}

	ImGui::Checkbox("postProccessEnable", &postProccessEnable_);
	EnumAdapter<SpriteLayer>::Combo("SpriteLayer", &layer_);
	ImGui::Separator();

	// 現在のlayerIndex_から「基準カテゴリ(base)」を復元
	SpriteLayerIndex base = SpriteLayerIndex::None;
	uint16_t baseVal = 0;
	for (uint32_t i = 0; i < EnumAdapter<SpriteLayerIndex>::GetEnumCount(); ++i) {

		SpriteLayerIndex v = EnumAdapter<SpriteLayerIndex>::GetValue(i);
		uint16_t vv = static_cast<uint16_t>(v);
		if (vv <= layerIndex_) {
			base = v;
			baseVal = vv;
		}
	}

	// 次のカテゴリ境界(>baseVal で最小の値)を探索
	uint16_t nextBoundary = (std::numeric_limits<uint16_t>::max)();
	for (uint32_t i = 0; i < EnumAdapter<SpriteLayerIndex>::GetEnumCount(); ++i) {
		uint16_t vv = static_cast<uint16_t>(EnumAdapter<SpriteLayerIndex>::GetValue(i));
		if (vv > baseVal && vv < nextBoundary) {
			nextBoundary = vv;
		}
	}
	uint16_t maxSub = (nextBoundary == (std::numeric_limits<uint16_t>::max)()) ?
		(std::numeric_limits<uint16_t>::max)() - baseVal :
		static_cast<uint16_t>(nextBoundary - baseVal - 1);

	// カテゴリ選択 + カテゴリ内順序
	bool changed = false;
	changed |= EnumAdapter<SpriteLayerIndex>::Combo("Layer Index", &base);

	// カテゴリ変更されたら基準値を更新して上限も再計算
	baseVal = static_cast<uint16_t>(base);
	nextBoundary = (std::numeric_limits<uint16_t>::max)();
	for (uint32_t i = 0; i < EnumAdapter<SpriteLayerIndex>::GetEnumCount(); ++i) {

		uint16_t vv = static_cast<uint16_t>(EnumAdapter<SpriteLayerIndex>::GetValue(i));
		if (vv > baseVal && vv < nextBoundary) {
			nextBoundary = vv;
		}
	}
	maxSub = (nextBoundary == (std::numeric_limits<uint16_t>::max)()) ?
		(std::numeric_limits<uint16_t>::max)() - baseVal :
		static_cast<uint16_t>(nextBoundary - baseVal - 1);

	int subInt = static_cast<int>(layerIndex_ - baseVal);
	changed |= ImGui::DragInt("Order Category", &subInt, 1.0f, 0, static_cast<int>(maxSub));
	if (changed) {
		if (subInt < 0) {

			subInt = 0;
		}
		if (subInt > static_cast<int>(maxSub)) {

			subInt = static_cast<int>(maxSub);
		}
		layerIndex_ = static_cast<uint16_t>(baseVal + static_cast<uint16_t>(subInt));
	}
	ImGui::SameLine();
	ImGui::TextDisabled("(abs: %u)", static_cast<unsigned>(layerIndex_));

	ImGui::Separator();

	EnumAdapter<BlendMode>::Combo("BlendMode", &blendMode_);

	ImGui::PopItemWidth();
}

void Sprite::ToJson(Json& data) {

	data["textureName"] = textureName_;
	data["postProccessEnable_"] = postProccessEnable_;
	data["layer"] = EnumAdapter<SpriteLayer>::ToString(layer_);
	data["layerIndex"] = layerIndex_;
	data["blendMode"] = EnumAdapter<BlendMode>::ToString(blendMode_);
}

void Sprite::FromJson(const Json& data) {

	postProccessEnable_ = data.value("postProccessEnable_", false);
	textureName_ = data["textureName"].get<std::string>();
	layer_ = EnumAdapter<SpriteLayer>::FromString(data["layer"].get<std::string>()).value();
	layerIndex_ = data["layerIndex"].get<uint16_t>();
	blendMode_ = EnumAdapter<BlendMode>::FromString(data["blendMode"].get<std::string>()).value();
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
