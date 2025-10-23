#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Raytracing/AccelerationStructureBuffer.h>
#include <Engine/Core/Graphics/Raytracing/RaytracingStructures.h>

//============================================================================
//	TopLevelAS class
//	TLAS(シーンインスタンス)の構築/更新と、インスタンス記述バッファ管理を行う。
//============================================================================
class TopLevelAS {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	TopLevelAS() = default;
	~TopLevelAS() = default;

	// インスタンス配列からTLASをビルドする(更新許可時はALLOW_UPDATE指定)
	void Build(ID3D12Device8* device, ID3D12GraphicsCommandList6* commandList,
		const std::vector<RayTracingInstance>& instances, bool allowUpdate);

	// 既存TLASをPERFORM_UPDATEで更新(インスタンス数変更時は再ビルド)
	void Update(ID3D12GraphicsCommandList6* commandList,
		const std::vector<RayTracingInstance>& instances);

	//--------- accessor -----------------------------------------------------

	// TLASリソースを取得する
	ID3D12Resource* GetResource() const { return result_.GetResource(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ID3D12Device8* device_;

	// スクラッチリソース
	AccelerationStructureBuffer instanceDescs_;
	AccelerationStructureBuffer scratch_;
	AccelerationStructureBuffer result_;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs_;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC  buildDesc_;

	bool allowUpdate_;

	//--------- functions ----------------------------------------------------

	// 4x4行列からDXRの3x4行列へ転置コピーする
	void CopyMatrix3x4(float(&dst)[3][4], const Matrix4x4& src);
};