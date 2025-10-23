#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Descriptors/BaseDescriptor.h>

//============================================================================
//	RTVDescriptor class
//	レンダーターゲットビュー(RTV)ディスクリプタを管理し、作成/参照を提供する。
//============================================================================
class RTVDescriptor :
	public BaseDescriptor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	RTVDescriptor() :BaseDescriptor(16) {};
	~RTVDescriptor() = default;

	// カラーリソースからRTVを作成しCPUハンドルを割り当てる。
	void Create(D3D12_CPU_DESCRIPTOR_HANDLE& handle, ID3D12Resource* resource,
		const D3D12_RENDER_TARGET_VIEW_DESC& desc);
};