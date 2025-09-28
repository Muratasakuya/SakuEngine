#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/ComPtr.h>

// directX
#include <d3d12.h>
#include <dxgi1_6.h>
// c++
#include <string>
#include <cassert>

//============================================================================
//	DxDevice class
//============================================================================
class DxDevice {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DxDevice() = default;
	~DxDevice() = default;

	void Create();

	//--------- accessor -----------------------------------------------------

	ID3D12Device8* Get() const { return device_.Get(); };

	IDXGIFactory7* GetDxgiFactory() const { return dxgiFactory_.Get(); };
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ComPtr<ID3D12Device8> device_;

	ComPtr<IDXGIFactory7> dxgiFactory_;

	ComPtr<IDXGIAdapter4> useAdapter_;

	//--------- functions ----------------------------------------------------

	std::string WStringToString(const std::wstring& wstr);
};