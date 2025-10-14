//============================================================================
//	include
//============================================================================

#include "../ParticleOutput.hlsli"
#include "../ParticleCommonSturctures.hlsli"
#include "../TrailParticle.hlsli"

//============================================================================
//	CBuffer
//============================================================================

struct Camera {
	
	float4x4 viewProjection;
};

ConstantBuffer<Camera> gCamera : register(b0);

//============================================================================
//	StructuredBuffer
//============================================================================

StructuredBuffer<TrailHeader> gTrailHeaders : register(t0);
StructuredBuffer<TrailVertex> gTrailVertices : register(t1);

//============================================================================
//	Main
//============================================================================
[numthreads(1, 1, 1)]
[outputtopology("triangle")]
void main(uint groupThreadId : SV_GroupThreadID, uint groupId : SV_GroupID,
out vertices MSOutput verts[TRAIL_MAX_VERTS], out indices uint3 polys[TRAIL_MAX_PRIMS]) {
	
	const uint instanceIndex = groupId.x;
	TrailHeader header = gTrailHeaders[instanceIndex];

	//  頂点、三角形出力すうを偶数に制限
	uint vertexCount = min(header.vertCount, TRAIL_MAX_VERTS);
	vertexCount &= ~1u;
	// 4頂点以下なら描画しないようにする
	if (vertexCount < 4) {

		vertexCount = 0;
	}
	uint primCount = (vertexCount == 0) ? 0 : (vertexCount - 2);
	if (TRAIL_MAX_PRIMS < primCount) {
		
		vertexCount = (TRAIL_MAX_PRIMS + 2) & ~1u;
		primCount = vertexCount - 2;
	}
	SetMeshOutputCounts(vertexCount, primCount);

	// なにも出力しないなら処理しない
	if (vertexCount == 0) {
		return;
	}

	const uint start = header.start;

	// 頂点書き込み
	for (uint i = 0; i < vertexCount; ++i) {
		
		TrailVertex vertex = gTrailVertices[start + i];
		MSOutput outVertex;
		outVertex.position = mul(float4(vertex.worldPos, 1.0f), gCamera.viewProjection);
		outVertex.texcoord = vertex.uv;
		outVertex.vertexColor = vertex.color;
		outVertex.instanceID = instanceIndex;
		verts[i] = outVertex;
	}

	// インデックス書き込み
	uint t = 0;
	for (uint i = 0; i + 3 < vertexCount; i += 2) {
		
		polys[t++] = uint3(i, i + 1, i + 2);
		polys[t++] = uint3(i + 1, i + 3, i + 2);
	}
}