//============================================================================
//	include
//============================================================================

#include "../ParticleOutput.hlsli"
#include "../ParticleCommonSturctures.hlsli"
#include "../TrailParticle.hlsli"

//============================================================================
//	CBuffer
//============================================================================

ConstantBuffer<PerView> gPerView : register(b0);

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
out vertices MSOutput verts[], out indices uint3 polys[]) {
	
	// dispatchMeshでの1次元グループID
	uint instanceIndex = groupId;
	// バッファアクセス
	TrailHeader trailHeader = gTrailHeaders[instanceIndex];
	
	uint vertexStart = trailHeader.start;
	uint vertCount = trailHeader.vertCount;
	// 4頂点以下なら出力しない
	if (vertCount < 4 || (vertCount & 1u)) {
		SetMeshOutputCounts(0, 0);
		return;
	}
	
	// 最大頂点数を超えないように制限
	const uint kMaxVertex = 256;
	vertCount = min(vertCount, kMaxVertex);
	// 出力三角形数
	uint polyCount = max(0, (int) vertCount - 2);
	// 頂点数を偶数に制限
	vertCount &= ~1u;
	polyCount = (4 <= vertCount) ? (vertCount - 2) : 0;
	
	// 頂点出力
	SetMeshOutputCounts(vertCount, polyCount);

	// 頂点
	for (uint i = 0; i < vertCount; ++i) {
		
		// バッファアクセス
		TrailVertex vertex = gTrailVertices[trailHeader.start + i];
		
		verts[i].position = mul(float4(vertex.worldPos, 1), gPerView.viewProjection);
		verts[i].texcoord = vertex.uv;
		verts[i].instanceID = instanceIndex;
		verts[i].vertexColor = vertex.color;
	}

	// インデックス
	uint t = 0;
	for (uint index = 0; index + 3 < vertCount; index += 2) {
		
		polys[t++] = uint3(index, index + 1, index + 2);
		polys[t++] = uint3(index + 1, index + 3, index + 2);
	}
}