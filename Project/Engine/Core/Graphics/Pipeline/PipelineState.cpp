#include "PipelineState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Asset/Filesystem.h>
#include <Engine/Core/Graphics/Pipeline/DxRootSignature.h>
#include <Engine/Core/Graphics/Pipeline/DxInputLayout.h>
#include <Engine/Core/Graphics/Pipeline/DxDepthRaster.h>
#include <Engine/Core/Graphics/Pipeline/DxBlendState.h>

//============================================================================
//	PipelineState classMethods
//============================================================================

void PipelineState::Create(const std::string& fileName, ID3D12Device8* device,
	SRVDescriptor* srvDescriptor, DxShaderCompiler* shaderCompiler) {

	// pipelineの種類
	PipelineType pipelineType{};

	// jsonFileの読み込み
	Json json = LoadFile(fileName);

	// shaderCompileを行う
	std::vector<ComPtr<IDxcBlob>> shaderBlobs;
	shaderCompiler->Compile(json, shaderBlobs);
	// shaderBlobsのサイズが1つならcomputeShaderとして処理をする
	if (shaderBlobs.size() == 1) {

		pipelineType = PipelineType::COMPUTE;
	} else {
		// meshShaderを使うかどうか
		if (json.contains("Type") && json["Type"] == "MS") {

			// typeがMS
			pipelineType = PipelineType::MESH;
		} else {

			// typeがMS
			pipelineType = PipelineType::VERTEX;
		}
	}

	// rootSignatureの作成
	DxRootSignature dxRootSignature;
	dxRootSignature.Create(fileName, json, device, srvDescriptor, rootSignature_);

	// pipelineごとの分岐処理
	// pipeline作成
	switch (pipelineType) {
	case PipelineState::PipelineType::VERTEX: {

		CreateVertexPipeline(fileName, json, device, shaderBlobs);
		break;
	}
	case PipelineState::PipelineType::MESH: {

		CreateMeshPipeline(fileName, json, device, shaderBlobs);
		break;
	}
	case PipelineState::PipelineType::COMPUTE: {

		CreateComputePipeline(fileName, device, shaderBlobs);
		break;
	}
	}
}

void PipelineState::CreateVertexPipeline(const std::string& fileName, const Json& json, ID3D12Device8* device,
	const std::vector<ComPtr<IDxcBlob>>& shaderBlobs) {

	// inputLayoutの作成
	DxInputLayout dxInputLayout;
	std::optional<D3D12_INPUT_LAYOUT_DESC> inputDesc;
	dxInputLayout.Create(json, inputDesc);

	// depth、rasterizerの作成
	DxDepthRaster depthRaster;
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	D3D12_DEPTH_STENCIL_DESC depthDesc{};
	depthRaster.Create(json, rasterizerDesc, depthDesc);

	// pipelinsStateの作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

	// 共通設定
	pipelineDesc.pRootSignature = rootSignature_.Get();
	pipelineDesc.VS = {
		.pShaderBytecode = shaderBlobs.front()->GetBufferPointer(),
		.BytecodeLength = shaderBlobs.front()->GetBufferSize()
	};
	pipelineDesc.PS = {
		.pShaderBytecode = shaderBlobs.back()->GetBufferPointer(),
		.BytecodeLength = shaderBlobs.back()->GetBufferSize()
	};
	pipelineDesc.RasterizerState = rasterizerDesc;
	pipelineDesc.DepthStencilState = depthDesc;
	pipelineDesc.SampleDesc.Count = 1;
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// inputLayoutがあれば設定する
	if (inputDesc.has_value()) {

		pipelineDesc.InputLayout = *inputDesc;
	} else {

		pipelineDesc.InputLayout.pInputElementDescs = nullptr;
		pipelineDesc.InputLayout.NumElements = 0;
	}

	const auto& stateJson = json["PipelineState"][0];
	const std::string topologyType = stateJson.value("TopologyType", "TRIANGLE");
	const std::string blendModeStr = stateJson.value("BlendMode", "NORMAL");

	if (topologyType == "TRIANGLE") {

		pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	} else if (topologyType == "LINE") {

		pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	}

	// DSV/RTV設定
	Formats formats = ParseFormatsFromJson(stateJson, [this](const std::string& string) { return GetFormatFromString(string); });
	pipelineDesc.DSVFormat = formats.dsv;
	for (UINT i = 0; i < formats.numRT; ++i) {

		pipelineDesc.RTVFormats[i] = formats.rtv[i];
	}
	pipelineDesc.NumRenderTargets = formats.numRT;
	pipelineDesc.BlendState.IndependentBlendEnable = formats.independentBlend;

	// pipelineの作成
	CreatePipelinesForBlendModes<D3D12_GRAPHICS_PIPELINE_STATE_DESC>(
		fileName, pipelineDesc, blendModeStr, formats,
		[&](BlendMode mode, D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc) -> HRESULT {

			graphicsPipelineStates_[mode] = nullptr;
			return device->CreateGraphicsPipelineState(
				&desc, IID_PPV_ARGS(&graphicsPipelineStates_[mode])); });
}


void PipelineState::CreateMeshPipeline(const std::string& fileName, const Json& json, ID3D12Device8* device,
	const std::vector<ComPtr<IDxcBlob>>& shaderBlobs) {

	// depth、rasterizerの作成
	DxDepthRaster depthRaster;
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	D3D12_DEPTH_STENCIL_DESC depthDesc{};
	depthRaster.Create(json, rasterizerDesc, depthDesc);

	// sampleDesc設定
	DXGI_SAMPLE_DESC sampleDesc{};
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;

	D3DX12_MESH_SHADER_PIPELINE_STATE_DESC pipelineDesc{};

	pipelineDesc.SampleMask = UINT_MAX;
	pipelineDesc.pRootSignature = rootSignature_.Get();

	// shaderByteCodeの設定
	pipelineDesc.MS = {
		shaderBlobs.front().Get()->GetBufferPointer(),
		shaderBlobs.front().Get()->GetBufferSize()
	};
	pipelineDesc.PS = {
		shaderBlobs.back().Get()->GetBufferPointer(),
		shaderBlobs.back().Get()->GetBufferSize()
	};

	const auto& stateJson = json["PipelineState"][0];
	const std::string blendModeStr = stateJson.value("BlendMode", "NORMAL");

	// DSV/RTVを設定
	Formats formats = ParseFormatsFromJson(stateJson, [this](const std::string& string) { return GetFormatFromString(string); });
	pipelineDesc.DSVFormat = formats.dsv;
	for (UINT i = 0; i < formats.numRT; ++i) {

		pipelineDesc.RTVFormats[i] = formats.rtv[i];
	}
	pipelineDesc.NumRenderTargets = formats.numRT;
	pipelineDesc.BlendState.IndependentBlendEnable = formats.independentBlend;
	pipelineDesc.RasterizerState = rasterizerDesc;
	pipelineDesc.DepthStencilState = depthDesc;
	pipelineDesc.SampleDesc = sampleDesc;

	// pipelineの作成
	CreatePipelinesForBlendModes<D3DX12_MESH_SHADER_PIPELINE_STATE_DESC>(
		fileName, pipelineDesc, blendModeStr, formats,
		[&](BlendMode mode, D3DX12_MESH_SHADER_PIPELINE_STATE_DESC& desc) -> HRESULT {

			CD3DX12_PIPELINE_MESH_STATE_STREAM pipelineStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(desc);
			D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
			streamDesc.pPipelineStateSubobjectStream = &pipelineStream;
			streamDesc.SizeInBytes = sizeof(pipelineStream);
			return device->CreatePipelineState(&streamDesc,
				IID_PPV_ARGS(graphicsPipelineStates_[mode].GetAddressOf())); });
}

void PipelineState::CreateComputePipeline(const std::string& fileName, ID3D12Device8* device,
	const std::vector<ComPtr<IDxcBlob>>& shaderBlobs) {

	// pipelinsStateの作成
	D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineDesc{};

	pipelineDesc.CS = {
	.pShaderBytecode = shaderBlobs.back()->GetBufferPointer(),
	.BytecodeLength = shaderBlobs.back()->GetBufferSize()
	};
	pipelineDesc.pRootSignature = rootSignature_.Get();

	// 生成
	HRESULT hr = device->CreateComputePipelineState(
		&pipelineDesc,
		IID_PPV_ARGS(&computePipelineState_));
	if (FAILED(hr)) {

		const std::string& file = "FileName: " + fileName + "\n";
		ASSERT(FALSE, file + "Filed create Pipeline");
	}
}

PipelineState::Formats PipelineState::ParseFormatsFromJson(const Json& stateJson,
	const std::function<DXGI_FORMAT(const std::string&)>& toFormat) {

	Formats formt{};

	// DSVの設定
	const std::string dsvFormat = stateJson.value("DSVFormat", "");
	if (dsvFormat == "D24_UNORM_S8_UINT") {

		formt.dsv = DXGI_FORMAT_D24_UNORM_S8_UINT;
	} else if (dsvFormat == "D32_FLOAT") {

		formt.dsv = DXGI_FORMAT_D32_FLOAT;
	}

	// RTVの設定
	const auto& rtvNode = stateJson["RTVFormats"];
	if (rtvNode.is_string()) {

		formt.rtv[0] = toFormat(rtvNode.get<std::string>());
		formt.numRT = 1;
	} else if (rtvNode.is_array()) {
		formt.numRT = (UINT)std::min<size_t>(rtvNode.size(), D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT);
		for (UINT i = 0; i < formt.numRT; ++i) {

			formt.rtv[i] = toFormat(rtvNode[i].get<std::string>());
		}
	}

	// RTVが2個以上ならブレンドをそれぞれ別々に設定する
	formt.independentBlend = (1 < formt.numRT);
	return formt;
}

void PipelineState::BuildBlendStateForMode(D3D12_BLEND_DESC& outDesc, BlendMode mode,
	const DXGI_FORMAT* rtvFormats, UINT numRT) {

	// ブレンドを作成
	DxBlendState dxBlendState;
	D3D12_RENDER_TARGET_BLEND_DESC blendDesc{};
	dxBlendState.Create(mode, blendDesc);

	// 既定値クリア
	outDesc = {};
	outDesc.IndependentBlendEnable = (1 < numRT) ? TRUE : FALSE;

	// カラーフォーマットかチェック
	auto IsColorFormat = [](DXGI_FORMAT format) {
		return (format == DXGI_FORMAT_R32G32B32A32_FLOAT) ||
			(format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB); };

	for (UINT i = 0; i < numRT; ++i) {
		if (IsColorFormat(rtvFormats[i])) {

			outDesc.RenderTarget[i] = blendDesc;
			outDesc.RenderTarget[i].BlendEnable = TRUE;
		} else {

			// 非カラー出力はブレンド無効
			outDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			outDesc.RenderTarget[i].BlendEnable = FALSE;
		}
	}
}

DXGI_FORMAT PipelineState::GetFormatFromString(const std::string& name) const {

	if (name == "R32G32B32A32_FLOAT") return DXGI_FORMAT_R32G32B32A32_FLOAT;
	if (name == "R32G32B32A32_UINT") return DXGI_FORMAT_R32G32B32A32_UINT;
	if (name == "R8G8B8A8_UNORM_SRGB") return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	if (name == "R32_FLOAT") return DXGI_FORMAT_R32_FLOAT;

	if (name == "R16_UNORM") return DXGI_FORMAT_R16_UNORM;
	if (name == "R8_UNORM") return DXGI_FORMAT_R8_UNORM;

	if (name == "R16_UINT") return DXGI_FORMAT_R16_UINT;
	if (name == "R8_UINT") return DXGI_FORMAT_R8_UINT;

	return DXGI_FORMAT_UNKNOWN;
}

ID3D12PipelineState* PipelineState::GetGraphicsPipeline(BlendMode blendMode) const {

	return graphicsPipelineStates_[blendMode].Get();
}

Json PipelineState::LoadFile(const std::string& fileName) {

	const fs::path basePath = "./Assets/Engine/ShaderData/";
	fs::path fullPath;

	if (!Filesystem::Found(basePath, fileName, fullPath)) {
		// ファイルが見つからなかった場合
		ASSERT(false, "Failed to find file: " + fileName);
	}

	std::ifstream file(fullPath);
	if (!file.is_open()) {
		ASSERT(false, "Failed to open file: " + fullPath.string());
	}

	Json json;
	file >> json;

	return json;
}