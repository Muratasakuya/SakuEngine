#pragma once

//============================================================================
//	include
//============================================================================

// directX
#include <d3d12.h>
// c++
#include <vector>
#include <memory>
#include <string>
#include <optional>
// json
#include <Externals/nlohmann/json.hpp>
// using
using Json = nlohmann::json;

//============================================================================
//	DxInputLayout class
//	頂点入力レイアウト(D3D12_INPUT_LAYOUT_DESC)をJSON定義から構築する。
//============================================================================
class DxInputLayout {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DxInputLayout() = default;
	~DxInputLayout() = default;

	// JSONの設定に基づき、Rasterizer/DepthStencil各記述を作成する
	void Create(const Json& json, std::optional<D3D12_INPUT_LAYOUT_DESC>& desc);
};