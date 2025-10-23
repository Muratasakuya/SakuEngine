#pragma once

//============================================================================
//	include
//============================================================================

// directX
#include <d3d12.h>
// c++
#include <cstdint>
// front
class DxCommand;

//============================================================================
//	MeshCommand class
// メッシュ描画処理を行う
//============================================================================
class MeshCommandContext {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	MeshCommandContext() = default;
	~MeshCommandContext() = default;

	// メッシュ、インスタンス数を取得して描画コマンドを発行
	void DispatchMesh(ID3D12GraphicsCommandList6* commandList,
		UINT instanceCount, uint32_t meshIndex, class IMesh* mesh);
	void DispatchMesh(ID3D12GraphicsCommandList6* commandList,
		UINT instanceCount, class EffectMesh* mesh);
};