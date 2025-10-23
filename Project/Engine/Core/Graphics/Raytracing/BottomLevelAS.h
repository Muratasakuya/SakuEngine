#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Raytracing/AccelerationStructureBuffer.h>

// c++
#include <vector>
// front
class IMesh;

//============================================================================
//	structure
//	BLAS構築要求。対象メッシュ/サブメッシュ/更新可否(skinnedか)を指定する。
//============================================================================

struct BuildRequest {

	IMesh* mesh;
	uint32_t meshIndex;
	bool allowUpdate; // skinnedMesh == true
};

//============================================================================
//	BottomLevelAS class
//	単一メッシュからBLASを構築・更新するユーティリティ。スキンメッシュはUpdate対応。
//============================================================================
class BottomLevelAS {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BottomLevelAS() = default;
	~BottomLevelAS() = default;

	// メッシュ情報からBLASをビルドする(必要なScratch/Resultを確保して実行)
	void Build(ID3D12Device8* device, ID3D12GraphicsCommandList6* commandList,
		const BuildRequest& request);
	
	// allowUpdate時に前フレームBLASをSourceとしてインクリメンタル更新する
	void Update(ID3D12GraphicsCommandList6* commandList);

	//--------- accessor -----------------------------------------------------

	// 生成済みBLASリソースを取得する
	ID3D12Resource* GetResource() const { return result_.GetResource(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// スクラッチリソース
	AccelerationStructureBuffer scratch_;
	AccelerationStructureBuffer result_;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs_{};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC  buildDesc_{};
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc_{};

	bool allowUpdate_;
};