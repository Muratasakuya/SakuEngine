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
	TrailHeader h = gTrailHeaders[instanceIndex];

    // 1) V/T を先に決める（偶数化・上限丸めもここで）
	uint V = min(h.vertCount, TRAIL_MAX_VERTS);
	V &= ~1u; // 偶数に
	if (V < 4)
		V = 0; // 小さすぎる場合は0で描かない
	uint T = (V == 0) ? 0 : (V - 2);
	if (T > TRAIL_MAX_PRIMS) {
		V = (TRAIL_MAX_PRIMS + 2) & ~1u;
		T = V - 2;
	}

    // 2) ここで一度だけ呼ぶ
	SetMeshOutputCounts(V, T);

    // 3) 出力なしなら終了（以降で verts/polys に触らない）
	if (V == 0)
		return;

	const uint start = h.start;

    // 頂点書き込み（0..V-1 だけ）
    [loop]
	for (uint i = 0; i < V; ++i) {
		TrailVertex v = gTrailVertices[start + i];
		MSOutput o;
		o.position = mul(float4(v.worldPos, 1.0f), gCamera.viewProjection);
		o.texcoord = v.uv;
		o.vertexColor = v.color;
		o.instanceID = instanceIndex;
		verts[i] = o;
	}

    // インデックス書き込み（0..T-1 だけ）
	uint t = 0;
    [loop]
	for (uint i = 0; i + 3 < V; i += 2) {
		polys[t++] = uint3(i, i + 1, i + 2);
		polys[t++] = uint3(i + 1, i + 3, i + 2);
	}
}