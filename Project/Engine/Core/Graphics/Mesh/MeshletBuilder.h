#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Mesh/MeshletStructures.h>
#include <Engine/Asset/AssetStructure.h>

// assimp
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
// meshoptimizer
#include <meshoptimizer.h>

//============================================================================
//	MeshletBuilder class
//	モデルデータからメッシュ/メッシュレットを生成し、頂点配列の最適化を行う。
//============================================================================
class MeshletBuilder {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	MeshletBuilder() = default;
	~MeshletBuilder() = default;

	// Assimpのシーンからメッシュを解析し、スキン指定に応じたResourceMeshを構築する
	ResourceMesh<MeshVertex> ParseMesh(const aiScene* scene, bool isSkinned);
	// 自前のModelDataからメッシュを解析してResourceMeshを構築する
	ResourceMesh<MeshVertex> ParseMesh(const ModelData& modelData);
	// ModelDataからエフェクト用メッシュを解析してResourceMeshを構築する
	ResourceMesh<EffectMeshVertex> ParseEffectMesh(const ModelData& modelData);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- functions ----------------------------------------------------

	// Assimpシーンから頂点配列/属性を構築して出力メッシュへ設定する
	void SetVertex(ResourceMesh<MeshVertex>& destinationMesh, const aiScene* scene);
	// ModelDataから頂点配列/属性を構築して出力メッシュへ設定する
	void SetVertex(ResourceMesh<MeshVertex>& destinationMesh, const ModelData& modelData);
	// ModelDataからエフェクト用頂点配列を構築して出力メッシュへ設定する
	void SetEffectVertex(ResourceMesh<EffectMeshVertex>& destinationMesh, const ModelData& modelData);

	// 頂点/インデックス並びを最適化してキャッシュ効率を高める
	void Optimize(ResourceMesh<MeshVertex>& destinationMesh);
	// エフェクト用メッシュの頂点/インデックスを最適化する
	void OptimizeEffectMesh(ResourceMesh<EffectMeshVertex>& destinationMesh);

	// メッシュレットを生成し、描画用のまとまりとして登録する
	void CreateMeshlet(ResourceMesh<MeshVertex>& destinationMesh);
	// エフェクト用メッシュレットを生成し、描画用のまとまりとして登録する
	void CreateEffectMeshlet(ResourceMesh<EffectMeshVertex>& destinationMesh);
};