#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/ComPtr.h>

// directX
#include <d3d12.h>
// c++
#include <vector>
#include <string>
// json
#include <Externals/nlohmann/json.hpp>
// using
using Json = nlohmann::json;

//============================================================================
//	DxRootSignature class
//============================================================================
class DxRootSignature {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DxRootSignature() = default;
	~DxRootSignature() = default;

	void Create(const std::string& fileName, const Json& json, ID3D12Device* device,
		class SRVDescriptor* srvDescriptor, ComPtr<ID3D12RootSignature>& rootSignature);
};