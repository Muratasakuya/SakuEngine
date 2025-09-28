#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/ComPtr.h>
#include <Engine/Core/Graphics/DxLib/DxStructures.h>
#include <Engine/Core/Graphics/Pipeline/DxShaderCompiler.h>

// directX
#include <d3d12.h>
#include <Externals/DirectX12/d3dx12.h>
// c++
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
// json
#include <Externals/nlohmann/json.hpp>
// using
using Json = nlohmann::json;

//============================================================================
//	PipelineState class
//============================================================================
class PipelineState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PipelineState() = default;
	virtual ~PipelineState() = default;

	void Create(const std::string& fileName, ID3D12Device8* device,
		class SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler);

	//--------- accessor -----------------------------------------------------

	ID3D12PipelineState* GetGraphicsPipeline(BlendMode blendMode = kBlendModeNormal) const;
	ID3D12PipelineState* GetComputePipeline() const { return computePipelineState_.Get(); };

	ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- enum class ---------------------------------------------------

	// pipelineの種類
	enum class PipelineType {

		VERTEX,
		MESH,
		COMPUTE
	};

	//--------- variables ----------------------------------------------------

	// blendModeの数だけ用意する
	std::array<ComPtr<ID3D12PipelineState>, BlendCount> graphicsPipelinepipelineStates_;

	ComPtr<ID3D12PipelineState> computePipelineState_;

	ComPtr<ID3D12RootSignature> rootSignature_;

	//--------- functions ----------------------------------------------------

	Json LoadFile(const std::string& fileName);

	void CreateVertexPipeline(const std::string& fileName, const Json& json, ID3D12Device8* device,
		const std::vector<ComPtr<IDxcBlob>>& shaderBlobs);

	void CreateMeshPipeline(const std::string& fileName, const Json& json, ID3D12Device8* device,
		const std::vector<ComPtr<IDxcBlob>>& shaderBlobs);

	void CreateComputePipeline(const std::string& fileName, ID3D12Device8* device,
		const std::vector<ComPtr<IDxcBlob>>& shaderBlobs);
};