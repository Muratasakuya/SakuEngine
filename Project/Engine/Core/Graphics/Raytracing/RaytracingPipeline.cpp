#include "RaytracingPipeline.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Pipeline/DxShaderCompiler.h>
#include <Engine/MathLib/Vector3.h>

//============================================================================
//	RaytracingPipeline classMethods
//============================================================================

void RaytracingPipeline::Init(ID3D12Device5* device, DxShaderCompiler* shaderCompiler) {

	//========================================================================
	// shaderのコンパイル処理

	ComPtr<IDxcBlob> missShaderBlob;
	shaderCompiler->CompileShader("ShadowRay_Miss.hlsl", L"./Assets/Engine/Shaders/DXR/ShadowRay_Miss.hlsl", L"lib_6_6", missShaderBlob, L"MissShadow");
	ComPtr<IDxcBlob> anyHitShaderBlob;
	shaderCompiler->CompileShader("ShadowRay_AnyHit.hlsl", L"./Assets/Engine/Shaders/DXR/ShadowRay_AnyHit.hlsl", L"lib_6_6", anyHitShaderBlob, L"AnyHitShadow");
	ComPtr<IDxcBlob> rayGenerationShaderBlob;
	shaderCompiler->CompileShader("ShadowRay_RayGeneration.hlsl", L"./Assets/Engine/Shaders/DXR/ShadowRay_RayGeneration.hlsl", L"lib_6_6", rayGenerationShaderBlob, L"RayGeneration");

	shaderCompiler->CompileShaderLibrary(L"./Assets/Engine/Shaders/DXR/ShadowRay_Miss.hlsl", L";", missShaderBlob);

	//========================================================================
	// DXILライブラリの設定

	const size_t exportCount = 3;
	const size_t subObjectCount = 7;

	std::vector<D3D12_EXPORT_DESC> exportList; exportList.reserve(exportCount);
	std::vector<D3D12_DXIL_LIBRARY_DESC> libList;   libList.reserve(exportCount);
	std::vector<D3D12_STATE_SUBOBJECT> subobjects; subobjects.reserve(subObjectCount);
	auto CreateLibrarySubobject = [](IDxcBlob* blob, LPCWSTR exportName, std::vector<D3D12_EXPORT_DESC>& exportList, std::vector<D3D12_DXIL_LIBRARY_DESC>& libList) -> D3D12_STATE_SUBOBJECT {
		exportList.push_back({ exportName, nullptr, D3D12_EXPORT_FLAG_NONE });

		D3D12_DXIL_LIBRARY_DESC libDesc{};
		libDesc.DXILLibrary.BytecodeLength = blob->GetBufferSize();
		libDesc.DXILLibrary.pShaderBytecode = blob->GetBufferPointer();
		libDesc.NumExports = 1;
		libDesc.pExports = &exportList.back();

		libList.push_back(libDesc);

		D3D12_STATE_SUBOBJECT subobject{};
		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		subobject.pDesc = &libList.back();
		return subobject;
		};
	// 各shaderを追加、作成
	// Miss
	subobjects.push_back(CreateLibrarySubobject(missShaderBlob.Get(), L"MissShadow", exportList, libList));
	// AnyHit
	subobjects.push_back(CreateLibrarySubobject(anyHitShaderBlob.Get(), L"AnyHitShadow", exportList, libList));
	// RayGeneration
	subobjects.push_back(CreateLibrarySubobject(rayGenerationShaderBlob.Get(), L"RayGeneration", exportList, libList));

	//========================================================================
	// HitGroupDescの設定

	D3D12_HIT_GROUP_DESC hitGroupDesc{};
	hitGroupDesc.HitGroupExport = L"AnyHitGroup";
	hitGroupDesc.AnyHitShaderImport = L"AnyHitShadow";
	hitGroupDesc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;

	subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &hitGroupDesc });

	//========================================================================
	// Global、LocalRootSignatureの設定

	// rootSignatureの作成
	globalRootSignature_ = CreateGlobalRootSignature(device);
	D3D12_GLOBAL_ROOT_SIGNATURE globalRSDesc{ globalRootSignature_.Get() };
	D3D12_STATE_SUBOBJECT globalRSSubobject{};
	globalRSSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
	globalRSSubobject.pDesc = &globalRSDesc;
	subobjects.push_back(globalRSSubobject);

	//========================================================================
	// Shader、PipelineConfigの設定

	// firstRay
	struct RadiancePayload {

		Vector3 color;
		float hitT;
		Vector3 worldPos;
		Vector3 worldNormal;
	};

	// shadowRay
	struct ShadowPayload {

		bool occluded;
	};

	D3D12_RAYTRACING_SHADER_CONFIG shaderConfig{};
	shaderConfig.MaxPayloadSizeInBytes = static_cast<UINT>((std::max)(sizeof(RadiancePayload), sizeof(ShadowPayload)));
	shaderConfig.MaxAttributeSizeInBytes = D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES;

	D3D12_STATE_SUBOBJECT shaderConfigSubobject{};
	shaderConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
	shaderConfigSubobject.pDesc = &shaderConfig;
	subobjects.push_back(shaderConfigSubobject);

	//========================================================================
	// pipelineの設定、作成

	// TraceRay呼び出し回数制限
	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig{};
	pipelineConfig.MaxTraceRecursionDepth = 1;

	D3D12_STATE_SUBOBJECT pipelineConfigSubobject{};
	pipelineConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
	pipelineConfigSubobject.pDesc = &pipelineConfig;
	subobjects.push_back(pipelineConfigSubobject);

	// pipeline作成
	D3D12_STATE_OBJECT_DESC stateObjectDesc{};
	stateObjectDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
	stateObjectDesc.NumSubobjects = static_cast<UINT>(subobjects.size());
	stateObjectDesc.pSubobjects = subobjects.data();

	HRESULT hr = device->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(&stateObject_));
	if (FAILED(hr)) {

		assert(SUCCEEDED(hr));
	}

	stateObject_->QueryInterface(IID_PPV_ARGS(&stateProps_));

	// shaderTableを構築
	BuildShaderTable(device);
}

ComPtr<ID3D12RootSignature> RaytracingPipeline::CreateGlobalRootSignature(ID3D12Device5* device) {

	CD3DX12_DESCRIPTOR_RANGE ranges[1]{};
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // u0

	CD3DX12_ROOT_PARAMETER rootParameters[3]{};
	rootParameters[0].InitAsShaderResourceView(0);          // SRV
	rootParameters[1].InitAsDescriptorTable(1, &ranges[0]); // UAVTable
	rootParameters[2].InitAsConstantBufferView(0);          // CBV

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = &rootParameters[0];
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	ComPtr<ID3DBlob> sigBlob;
	ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errorBlob);

	if (FAILED(hr)) {
		// エラー出力
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		return nullptr;
	}

	// 作成処理
	ComPtr<ID3D12RootSignature> rootSignature;
	device->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));

	return rootSignature;
}

void RaytracingPipeline::BuildShaderTable(ID3D12Device5* device) {

	// バッファサイズ
	shaderTableSize_ = 3 * kRecordStride; // 64×5

	// リソース作成
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(shaderTableSize_, D3D12_RESOURCE_FLAG_NONE);
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

	device->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&shaderTable_));

	uint8_t* pData = nullptr;
	shaderTable_->Map(0, nullptr, reinterpret_cast<void**>(&pData));

	// 0埋め
	std::memset(pData, 0, static_cast<size_t>(shaderTableSize_));

	// レコードを3つ配置
	std::memcpy(pData + kRayGenOffset, stateProps_->GetShaderIdentifier(L"RayGeneration"), kHandleSize);
	std::memcpy(pData + kMissOffset, stateProps_->GetShaderIdentifier(L"MissShadow"), kHandleSize);
	std::memcpy(pData + kHitGroupOffset, stateProps_->GetShaderIdentifier(L"AnyHitGroup"), kHandleSize);

	shaderTable_->Unmap(0, nullptr);
}