//============================================================================
//  include
//============================================================================
#include "../ParticleOutput.hlsli"
#include "../ParticleCommonSturctures.hlsli"
#include "../../../Math/Math.hlsli"

//============================================================================
//  Config
//============================================================================

// 出力上限
static const uint MAX_VERTS = 256; // 2 * nodeCount <= MAX_VERTS
static const uint MAX_PRIMS = 256; // 2 * (nodeCount - 1) <= MAX_PRIMS

// 1グループのスレッド数
#define MS_GROUP_SIZE 64

//============================================================================
//  CBuffer
//============================================================================

ConstantBuffer<PerView> gPerView : register(b0);

//============================================================================
//	StructuredBuffer
//============================================================================

struct Lightning {

	float3 start;
	float3 end;

	float width;
	uint nodeCount;

	float amplitudeRatio;
	float frequency;
	float smoothness;
	float time;

	float seed;
};

StructuredBuffer<Lightning> gLightnings : register(t0);
StructuredBuffer<Transform> gTransform : register(t1);

//============================================================================
//  Functions
//============================================================================

// ハッシュ関数
float hash11(float x) {
	
	return frac(sin(x * 12.9898f) * 43758.5453f);
}

// 進行方向から直交基底を作成
void MakeOrthoBasis(float3 dir, out float3 right, out float3 binormal) {
	
	float3 up = (abs(dir.y) > 0.95f) ? float3(1.0f, 0.0f, 0.0f) : float3(0.0f, 1.0f, 0.0f);
	right = normalize(cross(up, dir));
	binormal = normalize(cross(dir, right));
}

// 中心線雷のカーブを2D オフセットで歪ませる
float2 LightningOffset2D(float t, float seed, float amplitude, float frequency, float smoothness, float time) {
	
	// 端ほど弱く、中央に行くほど強い
	float edge = abs(2.0f * t - 1.0f);
	float falloff = pow(saturate(1.0f - edge), lerp(0.5f, 3.0f, saturate(smoothness)));
	// ねじれ位相
	float basePh = seed * 0.37f + time;
	float phase1 = (frequency * 6.2831853f) * t + basePh;
	float phase2 = (frequency * 0.73f * 6.2831853f) * t + basePh * 1.31f + 1.234f;
	// ノイズ＋正弦の合成
	float n1 = hash11(t * 157.0f + seed) * 2.0f - 1.0f;
	float n2 = hash11(t * 313.0f + seed) * 2.0f - 1.0f;
	float offsetX = (n1 * 0.5f + sin(phase1) * 0.5f) * amplitude * (0.3f + 0.7f * falloff);
	float offsetY = (n2 * 0.5f + cos(phase2) * 0.5f) * amplitude * (0.3f + 0.7f * falloff);
	return float2(offsetX, offsetY);
}

//============================================================================
//	Main
//============================================================================
[outputtopology("triangle")]
[numthreads(MS_GROUP_SIZE, 1, 1)]
void main(uint3 groupID : SV_GroupID,
uint3 groupThreadID : SV_GroupThreadID, uint groupIndex : SV_GroupIndex,
out vertices MSOutput verts[MAX_VERTS], out indices uint3 tris[MAX_PRIMS]) {

	// dispatchMeshでの1次元グループID
	const uint instanceIndex = groupID.x;
	// バッファアクセス
	Lightning lightning = gLightnings[instanceIndex];

	// ノード数、3以上に収める
	uint nodeCount = max(3, lightning.nodeCount);

	// 必要な出力サイズ
	// 左右2頂点/ノード
	uint requiredVerts = 2 * nodeCount;
	// 2三角形/セグメント
	uint requiredPrims = 2 * (nodeCount - 1);

	// 上限補正
	// 頂点数
	if (MAX_VERTS < requiredVerts) {
		
		nodeCount = MAX_VERTS / 2;
		requiredVerts = 2 * nodeCount;
		requiredPrims = 2 * (nodeCount - 1);
	}
	// プリミティブ数
	if (MAX_PRIMS < requiredPrims) {
		
		nodeCount = (MAX_PRIMS / 2) + 1;
		requiredVerts = 2 * nodeCount;
		requiredPrims = 2 * (nodeCount - 1);
	}

	// 出力頂点、プリミティブ数設定
	SetMeshOutputCounts(requiredVerts, requiredPrims);

	// ベースとなる方向と直交基底の計算
	float3 right, binormal;
	float3 baseVec = (lightning.end - lightning.start);
	float len = max(length(baseVec), 1e-5f);
	float3 direction = normalize(baseVec);
	MakeOrthoBasis(direction, right, binormal);

	// seedによる一定のねじれを与えて回転
	float twist = 6.2831853f * hash11(lightning.seed);
	float c = cos(twist);
	float s = sin(twist);
	float3 rightR = right * c + binormal * s;
	float3 binormalR = -right * s + binormal * c;

	// 頂点生成
	const float dt = 1.0f / float(nodeCount - 1);
	for (uint i = groupThreadID.x; i < nodeCount; i += MS_GROUP_SIZE) {

		float t01 = (nodeCount <= 1u) ? 0.0f : (float(i) * dt);

		// ワールド中心線
		float3 centerLocal = lerp(lightning.start, lightning.end, t01);
		// 距離に比例
		float amplitude = lightning.amplitudeRatio * len;
		float2 off2 = LightningOffset2D(t01, lightning.seed, amplitude,
		lightning.frequency, lightning.smoothness, lightning.time);

		// 2D オフセットを3Dに展開する
		float3 centerWorld = lerp(lightning.start, lightning.end, t01) + (rightR * off2.x + binormalR * off2.y);

		// ビルボード横方向: 視線 × 局所進行方向
		float3 toCam = normalize(gPerView.cameraPos - centerWorld);
		float3 side = normalize(cross(direction, toCam));
		if (any(isnan(side)) || length(side) < 1e-5f) {

			side = rightR;
		}

		// 端を細くする
		float edge = abs(2.0f * t01 - 1.0f);
		float taper = lerp(0.35f, 1.0f, 1.0f - edge);
		float halfWidth = lightning.width * 0.5f * taper;

		// 左右の頂点インデックス
		uint leftVertexIndex = 2u * i;
		// 右頂点は左頂点の+1番目
		uint rightVertexIndex = leftVertexIndex + 1;
		
		// 左右頂点
		float3 pL = centerWorld - side * halfWidth;
		float3 pR = centerWorld + side * halfWidth;

		// クリップ座標
		verts[leftVertexIndex].position = mul(float4(pL, 1.0f), gPerView.viewProjection);
		verts[rightVertexIndex].position = mul(float4(pR, 1.0f), gPerView.viewProjection);

		// UV
		verts[leftVertexIndex].texcoord = float2(0.0f, t01);
		verts[rightVertexIndex].texcoord = float2(1.0f, t01);

		// インデックスと頂点カラーは両方同じ
		verts[leftVertexIndex].vertexColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
		verts[leftVertexIndex].instanceID = instanceIndex;
		verts[rightVertexIndex].vertexColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
		verts[rightVertexIndex].instanceID = instanceIndex;
	}

	// インデックス生成
	uint segCount = nodeCount - 1;
	for (uint index = groupThreadID.x; index < segCount; index += MS_GROUP_SIZE) {
		
		uint leftI0 = 2 * index;
		// 右頂点は左頂点の+1番目
		uint rightI0 = leftI0 + 1;

		uint leftI1 = 2 * (index + 1);
		// 右頂点は左頂点の+1番目
		uint rightI1 = leftI1 + 1;

		uint triIndex = 2 * index;
		tris[triIndex] = uint3(leftI0, rightI0, leftI1);
		tris[triIndex + 1] = uint3(rightI0, rightI1, leftI1);
	}
}