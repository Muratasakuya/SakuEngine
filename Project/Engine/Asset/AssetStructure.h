#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/Transform.h>
#include <Engine/Core/Graphics/Mesh/MeshletStructures.h>
#include <Engine/Core/Graphics/DxLib/ComPtr.h>

// assimp
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
// directX
#include <d3d12.h>
// c++
#include <vector>
#include <array>
#include <string>
#include <unordered_map>
#include <optional>
#include <utility>
#include <span>
#include <map>

//============================================================================
//	Sprite
//============================================================================

struct SpriteVertexData {

	Vector2 pos;
	Vector2 texcoord;
	Color color;
};

//============================================================================
//	Model
//============================================================================

struct MeshModelData {

	std::vector<MeshVertex> vertices;
	std::vector<uint32_t> indices;
	std::optional<std::string> textureName;
	std::optional<std::string> normalMapTexture;
	std::optional<Color> baseColor;
};

struct Node {

	Transform3D transform;
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

template <typename tValue>
struct Keyframe {

	float time;
	tValue value;
};
using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;

template <typename tValue>
struct AnimationCurve {

	std::vector<Keyframe<tValue>> keyframes;
};
struct NodeAnimation {

	AnimationCurve<Vector3> scale;
	AnimationCurve<Quaternion> rotate;
	AnimationCurve<Vector3> translate;
};

//============================================================================
//	SkinnedMesh
//============================================================================

struct InputMeshVertex {

	std::vector<MeshVertex> data;
	uint32_t srvIndex;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> srvHandle;
};
struct OutputMeshVertex {

	std::vector<MeshVertex> data;
	uint32_t uavIndex;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> uavHandle;
};

struct VertexWeightData {

	float weight;
	uint32_t meshIndex;
	uint32_t vertexIndex;
};

struct JointWeightData {

	Matrix4x4 inverseBindPoseMatrix;
	std::vector<VertexWeightData> vertexWeights;
};

struct ModelData {

	std::vector<MeshModelData> meshes;
	std::map<std::string, JointWeightData> skinClusterData;
	Node rootNode;

	std::string fullPath;

	// 使用されたかどうか
	mutable bool isUse = false;
};

struct AnimationData {

	float duration;                                      // アニメーション全体の尺
	std::map<std::string, NodeAnimation> nodeAnimations; // NodeAnimationの集合、Node名で引けるようにしておく
};
struct Joint {

	Transform3D transform;
	bool isParentTransform;

	Matrix4x4 localMatrix;
	Matrix4x4 skeletonSpaceMatrix;   // skeletonSpaceでの変換行列
	std::string name;
	std::vector<int32_t> children;   // 子JointのIndexのリスト。いなければ空
	int32_t index;                   // 自身のIndex
	std::optional<int32_t> parent;   // 親JointのIndex。いなければnull
};
struct Skeleton {

	int32_t root;                            // RootJointのIndex
	std::map<std::string, int32_t> jointMap; // joint名とIndexの辞書
	std::vector<Joint> joints;               // 所属しているジョイント
	std::string name;
};

// 最大4Jointの影響を受ける
const uint32_t kNumMaxInfluence = 4;
struct VertexInfluence {

	std::array<float, kNumMaxInfluence> weights;
	std::array<int32_t, kNumMaxInfluence> jointIndices;
};
struct WellForGPU {

	Matrix4x4 skeletonSpaceMatrix;                 // 位置用
	Matrix4x4 skeletonSpaceInverseTransposeMatrix; // 法線用
};
struct SkinningInformation {

	uint32_t numVertices;
	uint32_t numBones;
};
struct SkinCluster {

	std::vector<Matrix4x4> inverseBindPoseMatrices;
	// インスタンシングに送るデータ
	std::vector<WellForGPU> mappedPalette;
};
struct AnimationBlendState {

	bool isBlending = false;    // ブレンド中か
	float blendTimer = 0.0f;    // 現在のブレンド経過時間
	float blendDuration = 0.0f; // ブレンドにかける総時間

	// 今まで再生していたアニメーション
	std::string fromAnimation;
	float fromAnimationTime = 0.0f;

	// 次に再生するアニメーション
	std::string toAnimation;
	float toAnimationTime = 0.0f;
};