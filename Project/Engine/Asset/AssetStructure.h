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
//	2Dスプライト描画で使用する頂点要素のセット(位置/UV/色)。
//============================================================================

struct SpriteVertexData {

	Vector2 pos;
	Vector2 texcoord;
	Color color;
};

//============================================================================
//	Model
//	静的モデルの構成要素と、階層・アニメーション記述をまとめる基礎データ群。
//============================================================================

//----------------------------------------------------------------------------
//	MeshModelData
//	1メッシュ分の頂点/インデックスと簡易マテリアル情報を保持する。
//----------------------------------------------------------------------------
struct MeshModelData {

	std::vector<MeshVertex> vertices;
	std::vector<uint32_t> indices;
	std::optional<std::string> textureName;
	std::optional<std::string> normalMapTexture;
	std::optional<Color> baseColor;
};

//----------------------------------------------------------------------------
//	Node
//	シーングラフのノード。SRT/ローカル行列/名称/子ノードを持つ。
//----------------------------------------------------------------------------
struct Node {

	Transform3D transform;
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

//----------------------------------------------------------------------------
//	Keyframe<tValue>
//	アニメーションの離散キー。時刻と値のペアを表す。
//----------------------------------------------------------------------------
template <typename tValue>
struct Keyframe {

	float time;
	tValue value;
};
using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;

//----------------------------------------------------------------------------
//	AnimationCurve<tValue>
//	単一チャンネルのカーブ(キー列)を表すテンプレート。
//----------------------------------------------------------------------------
template <typename tValue>
struct AnimationCurve {

	std::vector<Keyframe<tValue>> keyframes;
};

//----------------------------------------------------------------------------
//	NodeAnimation
//	1ノード分のSRTカーブセット。補間/適用時に合成して使用する。
//----------------------------------------------------------------------------
struct NodeAnimation {

	AnimationCurve<Vector3> scale;
	AnimationCurve<Quaternion> rotate;
	AnimationCurve<Vector3> translate;
};

//============================================================================
//	SkinnedMesh
//	GPUスキニング入出力バッファと、ボーン重み/スキンクラスタ関連の構造体群。
//============================================================================

//----------------------------------------------------------------------------
//	InputMeshVertex
//	スキニング入力(読み取り)用メッシュ頂点群とSRV参照を束ねる。
//----------------------------------------------------------------------------
struct InputMeshVertex {

	std::vector<MeshVertex> data;
	uint32_t srvIndex;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> srvHandle;
};

//----------------------------------------------------------------------------
//	OutputMeshVertex
//	スキニング結果(書き出し)用メッシュ頂点群とUAV参照を束ねる。
//----------------------------------------------------------------------------
struct OutputMeshVertex {

	std::vector<MeshVertex> data;
	uint32_t uavIndex;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> uavHandle;
};

//----------------------------------------------------------------------------
//	VertexWeightData
//	特定頂点に対する単一ジョイントの影響度を表す最小単位。
//----------------------------------------------------------------------------
struct VertexWeightData {

	float weight;
	uint32_t meshIndex;
	uint32_t vertexIndex;
};

//----------------------------------------------------------------------------
//	JointWeightData
//	1ジョイントが影響する頂点群と逆バインド行列を保持する。
//----------------------------------------------------------------------------
struct JointWeightData {

	Matrix4x4 inverseBindPoseMatrix;
	std::vector<VertexWeightData> vertexWeights;
};

//----------------------------------------------------------------------------
//	ModelData
//	モデル全体のメッシュ配列/スキン情報/階層ルート/ファイルパス等をまとめる。
//----------------------------------------------------------------------------
struct ModelData {

	std::vector<MeshModelData> meshes;
	std::map<std::string, JointWeightData> skinClusterData;
	Node rootNode;

	std::string fullPath;

	// 使用されたかどうか
	mutable bool isUse = false;
};

//----------------------------------------------------------------------------
//	AnimationData
//	アニメーション全体の尺と、各ノードのアニメーションカーブ集合を持つ。
//----------------------------------------------------------------------------
struct AnimationData {

	float duration;                                      // アニメーション全体の尺
	std::map<std::string, NodeAnimation> nodeAnimations; // NodeAnimationの集合、Node名で引けるようにしておく
};
//----------------------------------------------------------------------------
//	Joint
//	スケルトンの1関節を表す。局所/スケルトンスペース行列や親子関係を持つ。
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
//	Skeleton
//	スケルトン全体。ルート、ジョイント配列、名前→Index辞書を保持する。
//----------------------------------------------------------------------------
struct Skeleton {

	int32_t root;                            // RootJointのIndex
	std::map<std::string, int32_t> jointMap; // joint名とIndexの辞書
	std::vector<Joint> joints;               // 所属しているジョイント
	std::string name;
};

// 最大4Jointの影響を受ける
const uint32_t kNumMaxInfluence = 4;
//----------------------------------------------------------------------------
//	VertexInfluence
//	1頂点が参照するジョイントIndexと重みの固定長バッファ。
//----------------------------------------------------------------------------
struct VertexInfluence {

	std::array<float, kNumMaxInfluence> weights;
	std::array<int32_t, kNumMaxInfluence> jointIndices;
};

//----------------------------------------------------------------------------
//	WellForGPU
//	GPUパレットに渡す行列セット。位置用/法線用で別行列を持つ。
//----------------------------------------------------------------------------
struct WellForGPU {

	Matrix4x4 skeletonSpaceMatrix;                 // 位置用
	Matrix4x4 skeletonSpaceInverseTransposeMatrix; // 法線用
};

//----------------------------------------------------------------------------
//	SkinningInformation
//	スキニング対象の頂点数/ボーン数など規模情報をまとめる。
//----------------------------------------------------------------------------
struct SkinningInformation {

	uint32_t numVertices;
	uint32_t numBones;
};

//----------------------------------------------------------------------------
//	SkinCluster
//	スキニングで使用する逆バインド行列配列と、描画時に参照するパレット。
//----------------------------------------------------------------------------
struct SkinCluster {

	std::vector<Matrix4x4> inverseBindPoseMatrices;
	// インスタンシングに送るデータ
	std::vector<WellForGPU> mappedPalette;
};

//----------------------------------------------------------------------------
//	AnimationBlendState
//	アニメーションのブレンド進行状態と、from/toアニメの再生位置を管理する。
//----------------------------------------------------------------------------
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