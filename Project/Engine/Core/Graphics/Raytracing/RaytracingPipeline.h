#pragma once

//============================================================================
//	define
//============================================================================
#define ALIGN(value, alignment) (((value) + ((alignment) - 1)) & ~((alignment) - 1))

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/ComPtr.h>

// directX
#include <d3d12.h>
#include <Externals/DirectX12/d3dx12.h>
// c++
#include <array>
#include <vector>
// front
class DxShaderCompiler;

//============================================================================
//	RaytracingPipeline class
//============================================================================
class RaytracingPipeline {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	static constexpr UINT64 kHandleSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES; // 32
	static constexpr UINT64 kRecordStride = ALIGN(kHandleSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT); // 32
	static constexpr UINT64 kTableAlign = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT; // 64

	static constexpr UINT64 kRayGenOffset = 0 * kTableAlign;   // 0
	static constexpr UINT64 kMissOffset = 1 * kTableAlign;     // 64
	static constexpr UINT64 kHitGroupOffset = 2 * kTableAlign; // 128
public:
	//========================================================================
	//	public Methods
	//========================================================================

	RaytracingPipeline() = default;
	~RaytracingPipeline() = default;

	void Init(ID3D12Device5* device, DxShaderCompiler* shaderCompiler);

	//--------- accessor -----------------------------------------------------

	ID3D12StateObject* GetPipelineState() const { return stateObject_.Get(); }
	ID3D12RootSignature* GetRootSignature() const { return globalRootSignature_.Get(); }
	ID3D12Resource* GetShaderTable() const { return shaderTable_.Get(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// pipeline
	ComPtr<ID3D12StateObject> stateObject_;
	ComPtr<ID3D12RootSignature> globalRootSignature_;

	ComPtr<ID3D12StateObjectProperties> stateProps_;

	// shaderTable
	ComPtr<ID3D12Resource> shaderTable_;
	UINT shaderTableSize_;

	//--------- functions ----------------------------------------------------

	ComPtr<ID3D12RootSignature> CreateGlobalRootSignature(ID3D12Device5* device);

	void BuildShaderTable(ID3D12Device5* device);
};