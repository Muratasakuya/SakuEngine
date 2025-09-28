#include "RTVDescriptor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/DxUtils.h>
#include <Engine/Core/Debug/Assert.h>

//============================================================================
//	RTVDescriptor classMethods
//============================================================================

void RTVDescriptor::Create(D3D12_CPU_DESCRIPTOR_HANDLE& handle, ID3D12Resource* resource,
	const D3D12_RENDER_TARGET_VIEW_DESC& desc) {

	uint32_t index = Allocate();
	// 参照で飛び出し側のhandleも設定する
	handle = GetCPUHandle(index);
	device_->CreateRenderTargetView(resource, &desc, handle);
}