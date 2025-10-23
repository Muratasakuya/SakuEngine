#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/ComPtr.h>
#include <Engine/Core/Graphics/DxLib/DxStructures.h>
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Core/Graphics/Pipeline/DxShaderCompiler.h>
#include <Engine/Utility/Helper/Algorithm.h>

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
//	JSON定義からGraphics/ComputeのPSOとRootSignatureを構築・保持し、ブレンド別PSO取得APIを提供する。
//============================================================================
class PipelineState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PipelineState() = default;
	virtual ~PipelineState() = default;

	// JSONを読み込み、シェーダをコンパイルし、RootSignature/各種PSOを生成する
	void Create(const std::string& fileName, ID3D12Device8* device,
		class SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler);

	//--------- accessor -----------------------------------------------------

	// ブレンドモードに対応するGraphics PSOを取得する
	ID3D12PipelineState* GetGraphicsPipeline(BlendMode blendMode = BlendMode::kBlendModeNormal) const;
	// Compute PSOを取得する
	ID3D12PipelineState* GetComputePipeline() const { return computePipelineState_.Get(); };
	// 生成済みRootSignatureを取得する
	ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// pipelineの種類
	enum class PipelineType {

		VERTEX,
		MESH,
		COMPUTE
	};

	// viewのタイプ
	struct Formats {

		DXGI_FORMAT dsv = DXGI_FORMAT_UNKNOWN;
		DXGI_FORMAT rtv[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT]{};

		UINT numRT = 0;
		bool independentBlend = false;
	};

	//--------- variables ----------------------------------------------------

	// blendModeの数だけ用意する
	std::array<ComPtr<ID3D12PipelineState>, kBlendModeCount> graphicsPipelineStates_;

	ComPtr<ID3D12PipelineState> computePipelineState_;

	ComPtr<ID3D12RootSignature> rootSignature_;

	//--------- functions ----------------------------------------------------

	// ShaderData(JSON)を読み込む
	Json LoadFile(const std::string& fileName);

	// VS/PSパイプラインを構築し、必要なブレンドモード分のPSOを生成する
	void CreateVertexPipeline(const std::string& fileName, const Json& json, ID3D12Device8* device,
		const std::vector<ComPtr<IDxcBlob>>& shaderBlobs);
	// MeshShaderパイプラインを構築し、必要なブレンドモード分のPSOを生成する
	void CreateMeshPipeline(const std::string& fileName, const Json& json, ID3D12Device8* device,
		const std::vector<ComPtr<IDxcBlob>>& shaderBlobs);
	// Computeパイプラインを構築する
	void CreateComputePipeline(const std::string& fileName, ID3D12Device8* device,
		const std::vector<ComPtr<IDxcBlob>>& shaderBlobs);

	// 文字列からDXGI_FORMATへ変換する
	DXGI_FORMAT GetFormatFromString(const std::string& name) const;
	// JSONからDSV/RTVフォーマット等を抽出してFormatsを組み立てる
	Formats ParseFormatsFromJson(const Json& stateJson,
		const std::function<DXGI_FORMAT(const std::string&)>& toFormat);
	// ブレンドモード別にD3D12_BLEND_DESCを構築する
	void BuildBlendStateForMode(D3D12_BLEND_DESC& outDesc, BlendMode mode,
		const DXGI_FORMAT* rtvFormats, UINT numRT);

	// ブレンドモード毎にPSOを生成する(共通テンプレート)
	template<typename Desc, typename CreateOneFn>
	void CreatePipelinesForBlendModes(const std::string& fileName, Desc& baseDesc,
		const std::string& blendModeString, const Formats& formats, CreateOneFn&& createOne);
};

// ブレンドモード毎にPSOを生成する共通テンプレートの実装
template<typename Desc, typename CreateOneFn>
inline void PipelineState::CreatePipelinesForBlendModes(const std::string& fileName,
	Desc& baseDesc, const std::string& blendModeString, const Formats& formats, CreateOneFn&& createOne) {

	// pipelineを作成する
	const auto Make = [&](BlendMode mode) {

		// ブレンドデスクを差し替えてから生成
		BuildBlendStateForMode(baseDesc.BlendState, mode, formats.rtv, formats.numRT);
		HRESULT hr = createOne(mode, baseDesc);
		if (FAILED(hr)) {

			const std::string& file = "FileName: " + fileName + "\n";
			ASSERT(FALSE, file + "Filed create Pipeline");
		}};

	// 全てのブレンドで作成
	if (blendModeString == "ALL") {
		for (const auto& mode : Algorithm::GetEnumArray(BlendMode::kBlendModeCount)) {

			Make(static_cast<BlendMode>(mode));
		}
	} else {

		// Normalのみ
		Make(BlendMode::kBlendModeNormal);
	}
}