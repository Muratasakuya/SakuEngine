#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Raytracing/BottomLevelAS.h>
#include <Engine/Core/Graphics/Raytracing/TopLevelAS.h>

// c++
#include <unordered_map>
#include <memory>

//============================================================================
//	RaytracingScene class
//	全メッシュのBLAS群とシーン全体のTLASを管理し、初回ビルド/更新を分岐して実行する。
//============================================================================
class RaytracingScene {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	RaytracingScene() = default;
	~RaytracingScene() = default;

	// デバイスを受け取り初期状態にセットする
	void Init(ID3D12Device8* device);

	// 受け取ったメッシュ群に対してBLASを構築/必要なら更新する
	void BuildBLASes(ID3D12GraphicsCommandList6* commandList, const std::vector<IMesh*>& meshes);
	// インスタンス配列からTLASを構築(初回はBuild/以降はUpdate)する
	void BuildTLAS(ID3D12GraphicsCommandList6* commandList,
		const std::vector<RayTracingInstance>& instances);

	//--------- accessor -----------------------------------------------------

	// 特定メッシュのBLASを取得/現在のTLASを取得
	ID3D12Resource* GetBLASResource(IMesh* mesh, uint32_t meshCount) const;
	ID3D12Resource* GetTLASResource() const { return tlas_.GetResource(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// BLASをメッシュ×サブメッシュで識別するためのキー
	struct MeshKey {
		IMesh* ptr;
		uint32_t subMesh;
		bool operator==(const MeshKey&) const = default;
	};
	// MeshKey用のハッシュ
	struct MeshKeyHash {
		size_t operator()(const MeshKey& k) const noexcept {
			return std::hash<IMesh*>{}(k.ptr) ^ (k.subMesh * 0x9e3779b97f4a7c15ULL);
		}
	};

	//--------- variables ----------------------------------------------------

	ID3D12Device8* device_;

	std::unordered_map<MeshKey, BottomLevelAS, MeshKeyHash> blases_;
	TopLevelAS tlas_;

	bool first_;
};