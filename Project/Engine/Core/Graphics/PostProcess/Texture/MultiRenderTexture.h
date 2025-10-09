#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Texture/RenderTexture.h>

// c++
#include <vector>

//============================================================================
//	MultiRenderTexture class
//============================================================================
class MultiRenderTexture {
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 使用するRTV
	struct Attachment {

		std::unique_ptr<RenderTexture> texture; // 既存RTを使う（当面はRTV=SRV同フォーマット）
		RenderTarget renderTarget;              // テクスチャ情報
	};
public:
	//========================================================================
	//	public Methods
	//========================================================================

	MultiRenderTexture() = default;
	~MultiRenderTexture() = default;

	void Init(ID3D12Device* device, RTVDescriptor* rtvDescriptor,
		SRVDescriptor* srvDescriptor, uint32_t width, uint32_t height);

	// RTVの追加
	void AddColorTarget(DXGI_FORMAT format, const Color& color);
	void AddMaskTarget(DXGI_FORMAT format);

	//--------- accessor -----------------------------------------------------

	RenderTexture* GetRenderTexture(uint32_t i) const { return attachments_[i].texture.get(); }

	const std::vector<Attachment>& GetAttachments() const { return attachments_; }
	const Attachment& GetPrimary()  const { return attachments_[primary_]; }

	uint32_t GetWidth() const { return width_; }
	uint32_t GetHeight() const { return height_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ID3D12Device* device_;
	RTVDescriptor* rtvDescriptor_;
	SRVDescriptor* srvDescriptor_;

	// 使用するRTV
	std::vector<Attachment> attachments_;

	uint32_t width_;
	uint32_t height_;
	size_t primary_ = 0;

	//--------- functions ----------------------------------------------------

	// helper
	void AddAttachment(DXGI_FORMAT format, const Color& color);
};