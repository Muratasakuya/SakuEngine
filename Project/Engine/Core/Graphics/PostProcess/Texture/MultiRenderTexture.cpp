#include "MultiRenderTexture.h"

//============================================================================
//	MultiRenderTexture classMethods
//============================================================================

void MultiRenderTexture::Init(ID3D12Device* device, RTVDescriptor* rtvDescriptor,
	SRVDescriptor* srvDescriptor, uint32_t width, uint32_t height) {

	device_ = nullptr;
	device_ = device;

	rtvDescriptor_ = nullptr;
	rtvDescriptor_ = rtvDescriptor;

	srvDescriptor_ = nullptr;
	srvDescriptor_ = srvDescriptor;

	width_ = width;
	height_ = height;
}

void MultiRenderTexture::AddColorTarget(DXGI_FORMAT format, const Color& color) {

	AddAttachment(format, color);
	primary_ = 0;
}

void MultiRenderTexture::AddMaskTarget(DXGI_FORMAT format) {

	AddAttachment(format, Color::Black(0.0f));
}

void MultiRenderTexture::AddAttachment(DXGI_FORMAT format, const Color& color) {

	// RTVテクスチャの作成
	auto texture = std::make_unique<RenderTexture>();
	texture->Create(width_, height_, color, format, device_, rtvDescriptor_, srvDescriptor_);

	// RTVテクスチャを追加
	Attachment attachment{};
	attachment.renderTarget = texture->GetRenderTarget();
	attachment.texture = std::move(texture);
	attachments_.emplace_back(std::move(attachment));
}