#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/MathUtils.h>

//c++
#include <cstdint>
#include <vector>

//============================================================================
//	MeshletStructures
//============================================================================

struct MeshInstanceData {

	uint32_t meshletCount;
	uint32_t numVertices;
	int32_t isSkinned;
};
struct EffectMeshInstanceData {

	uint32_t meshletCount;
	uint32_t numVertices;
};

// mesh頂点情報
struct MeshVertex {

	Vector4 pos;
	Vector2 texcoord;
	Vector3 normal;
	Color color;
	Vector3 tangent;
	Vector3 bitangent;
};
struct EffectMeshVertex {

	Vector4 pos;
	Vector2 texcoord;
	Color color;
};

// meshlet情報の格納
struct ResourceMeshlet {

	// vertex
	uint32_t vertexOffset; // 頂点番号オフセット
	uint32_t vertexCount;  // 頂点数

	// primitive
	uint32_t primitiveOffset; // プリミティブ番号オフセット
	uint32_t primitiveCount;  // プリミティブオフセット

	Color color; // meshletの色
};

// 出力index用
struct ResourcePrimitiveIndex {

	uint32_t index0 : 10; // 出力頂点番号0、10bit
	uint32_t index1 : 10; // 出力頂点番号1、10bit
	uint32_t index2 : 10; // 出力頂点番号2、10bit

	uint32_t reserved : 2; // 予約領域
};

// 上記のデータを格納
template<typename T>
struct ResourceMesh {

	// mesh数
	size_t meshCount_;

	std::vector<std::vector<T>> vertices;
	std::vector<std::vector<uint32_t>> indices;

	std::vector<std::vector<ResourceMeshlet>> meshlets;
	std::vector<std::vector<uint32_t>> uniqueVertexIndices;
	std::vector<std::vector<ResourcePrimitiveIndex>> primitiveIndices;

	bool isSkinned;

	// operator
	ResourceMesh<T>& operator=(const ResourceMesh<T>& other) {
		if (this != &other) {
			meshCount_ = other.meshCount_;
			vertices = other.vertices;
			indices = other.indices;
			meshlets = other.meshlets;
			uniqueVertexIndices = other.uniqueVertexIndices;
			primitiveIndices = other.primitiveIndices;
		}
		return *this;
	}
};