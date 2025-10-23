#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/GPUObject/CBufferStructures.h>

// directX
#include <d3d12.h>

//============================================================================
//	RayTracingStructures
//	レイトレーシングでGPUへ渡す構造体定義をまとめる(TLASインスタンス/レイ共通パラメータ)。
//============================================================================

//----------------------------------------------------------------------------
//	RayTracingInstance
//	TLASのD3D12_RAYTRACING_INSTANCE_DESCに対応するアプリ側インスタンス情報。
//----------------------------------------------------------------------------
struct RayTracingInstance {

	Matrix4x4 matrix;
	UINT instanceID : 24;  // SV_InstanceID
	UINT mask : 8;         // レイマスク
	UINT hitGroupIdx : 24; // ShaderTableのHitGroupインデックス
	UINT flags : 8;        // D3D12_RAYTRACING_INSTANCE_FLAG_*
	ID3D12Resource* blas;  // 対応するBLAS
};

//----------------------------------------------------------------------------
//	RaySceneForGPU
//	レイの射出範囲など、DXRで共通利用するパラメータをまとめた定数バッファ相当。
//----------------------------------------------------------------------------
struct RaySceneForGPU {

	// ray
	float rayMin; // 飛ばす最小位置
	float rayMax; // 飛ばす距離
};