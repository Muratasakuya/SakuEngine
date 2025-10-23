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
//	DXGIファクトリ/アダプタ選定とD3D12デバイス生成を行い、デバイス取得を提供する。
//============================================================================
class DxDevice {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DxDevice() = default;
	~DxDevice() = default;

	// DXGIファクトリ作成→アダプタ列挙/選定→D3D12デバイス生成を行う
	void Create();

	//--------- accessor -----------------------------------------------------

	// デバイスを取得する
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

	// 内部ヘルパ: ワイド文字列をUTF-8へ変換する
	std::string WStringToString(const std::wstring& wstr);
};