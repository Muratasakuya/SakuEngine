//============================================================================
//	include
//============================================================================

#include "../ParticleOutput.hlsli"
#include "../ParticleCommonSturctures.hlsli"

#include "../../../Math/Math.hlsli"

//============================================================================
// Constant
//============================================================================

#ifndef CRESCENT_MAX_DIVIDE
#define CRESCENT_MAX_DIVIDE 30
#endif

#ifndef CRESCENT_WEIGHT_SCALE
#define CRESCENT_WEIGHT_SCALE 0.5f
#endif

//============================================================================
//	CBuffer
//============================================================================

ConstantBuffer<PerView> gPerView : register(b0);

//============================================================================
//	StructuredBuffer
//============================================================================

struct Crescent {
	
	// 半径
	float outerRadius;
	float innerRadius;
	
	// 始点と終点
	float startAngle;
	float endAngle;
	
	// 変形
	float thickness;
	float2 pivot;
	
	// 頂点色
	float4 outerColor;
	float4 innerColor;
	
	// 端の尖り度
	float2 tipSharpness;
	
	// 孤の重心
	float2 weight;
	
	uint divide;
	uint uvMode;
};

StructuredBuffer<Crescent> gCrescents : register(t0);
StructuredBuffer<Transform> gTransform : register(t1);

//============================================================================
//	Functions
//============================================================================

float2 ArcPoint(const Crescent crescent, float t, bool outer) {
	
	// 角度
	float angRad = lerp(crescent.startAngle, crescent.endAngle, t);

	// 端の尖り度
	float tip = lerp(crescent.tipSharpness.x, crescent.tipSharpness.y, t);
	tip = saturate(tip);
	float taper = lerp(1.0f, sin(t * PI), tip);

	// 半径
	float thicknessR = crescent.outerRadius - crescent.innerRadius;
	float innerDynamic = crescent.outerRadius - thicknessR * taper;
	float r = outer ? crescent.outerRadius : innerDynamic;

	// 円弧の基準位置
	float2 dir = float2(cos(angRad), sin(angRad));
	float2 pos = dir * r;

	// 接線方向
	float2 tangent = float2(-dir.y, dir.x);

	// weightで重い側へ伸ばす
	float wdiff = crescent.weight.y - crescent.weight.x;
	float wamount = wdiff * CRESCENT_WEIGHT_SCALE * thicknessR * sin(t * PI);

	// 接線方向に押し出し
	pos += tangent * wamount;

	return pos;
}

//============================================================================
//	Main
//============================================================================
[numthreads(CRESCENT_MAX_DIVIDE * 4, 1, 1)]
[outputtopology("triangle")]
void main(uint groupThreadId : SV_GroupThreadID, uint groupId : SV_GroupID,
out vertices MSOutput verts[(CRESCENT_MAX_DIVIDE + 1) * 4], out indices uint3 polys[(CRESCENT_MAX_DIVIDE * 6) + 4]) {
	
	// dispatchMeshでの1次元グループID
	uint instanceIndex = groupId;
	// バッファアクセス
	Crescent crescent = gCrescents[instanceIndex];
	Transform transform = gTransform[instanceIndex];
	
	// 頂点数、出力三角形数
	uint divide = min(crescent.divide, (uint) CRESCENT_MAX_DIVIDE);
	uint vertCount = (divide + 1) * 4;
	uint triCount = divide * 4 + 4;
	SetMeshOutputCounts(vertCount, triCount);

	if (groupThreadId < vertCount) {
		
		uint segIndex = groupThreadId >> 2;
		uint localIdx = groupThreadId & 3;
		bool outer = (localIdx == 0 || localIdx == 2);
		bool frontSide = (localIdx <= 1);

		float t = segIndex / (float) divide;
		float2 pos2 = ArcPoint(crescent, t, outer);

		// ピボットでオフセットを設定
		float2 halfExtent = float2(crescent.outerRadius, crescent.outerRadius);
		float2 pivotOff = lerp(-halfExtent, halfExtent, crescent.pivot);
		pos2 -= pivotOff;

		// 幅設定
		float halfT = 0.5f * crescent.thickness;
		// 端(start/end)で0、中央で1になるスケール
		float tip = lerp(crescent.tipSharpness.x, crescent.tipSharpness.y, t);
		tip = saturate(tip);
		float taper = lerp(1.0f, sin(t * PI), tip);
		float z = (outer ? 0.0f : (frontSide ? halfT * taper : -halfT * taper));

		// world行列を作成
		float4x4 worldMatrix = MakeWorldMatrix(transform, gPerView.billboardMatrix, gPerView.cameraPos);
		worldMatrix[3].xyz = transform.translation;
		
		// 親の設定
		worldMatrix = SetParent(transform, worldMatrix);

		// 行列計算
		float4x4 wvp = mul(worldMatrix, gPerView.viewProjection);
		float4 pos = mul(float4(pos2, z, 1.0f), wvp);

		// 頂点情報を設定
		MSOutput vertex;
		vertex.position = pos;
		vertex.instanceID = instanceIndex;

		// UV設定
		// 前面
		if (frontSide) {
			
			vertex.texcoord = (crescent.uvMode == 0) ? float2(t, outer ? 0.0f : 1.0f) : float2(outer ? 0.0f : 1.0f, t);
		}
		// 背面
		else {
			
			vertex.texcoord = (crescent.uvMode == 0) ? float2(t, outer ? 0.0f : 1.0f) : float2(outer ? 0.0f : 1.0f, t);
		}
		
		// 頂点色
		float radialWeight = outer ? 1.0f : 0.0f;
		vertex.vertexColor = lerp(crescent.innerColor, crescent.outerColor, radialWeight);

		verts[groupThreadId] = vertex;
	}

	if (groupThreadId < divide) {
		
		uint i = groupThreadId;
		uint index0 = i * 4;
		uint index1 = (i + 1) * 4;

		// 各頂点index
		uint OF0 = index0 + 0; // Outer Front
		uint IF0 = index0 + 1; // Inner Front
		uint OB0 = index0 + 2; // Outer Back
		uint IB0 = index0 + 3; // Inner Back

		uint OF1 = index1 + 0;
		uint IF1 = index1 + 1;
		uint OB1 = index1 + 2;
		uint IB1 = index1 + 3;

		uint baseFront = 0;
		uint baseBack = baseFront + divide * 2;

		//  +Z
		uint fIndex = baseFront + i * 2;
		polys[fIndex + 0] = uint3(OF0, IF0, OF1);
		polys[fIndex + 1] = uint3(OF1, IF0, IF1);

		// -Z
		uint bIndex = baseBack + i * 2;
		polys[bIndex + 0] = uint3(OB1, IB0, OB0);
		polys[bIndex + 1] = uint3(OB1, IB1, IB0);
	}

	// 始点と終点の三角形生成
	if (groupThreadId == 0) {

		uint polyIndex = divide * 4;

		uint OF0 = 0;
		uint IF0 = 1;
		uint OB0 = 2;
		uint IB0 = 3;

		uint OFN = polyIndex + 0;
		uint IFN = polyIndex + 1;
		uint OBN = polyIndex + 2;
		uint IBN = polyIndex + 3;

		uint capBase = divide * 4;

		// startAngle
		polys[capBase + 0] = uint3(OF0, OB0, IF0);
		polys[capBase + 1] = uint3(IF0, OB0, IB0);

		// endAngle
		polys[capBase + 2] = uint3(OFN, IFN, OBN);
		polys[capBase + 3] = uint3(IFN, IBN, OBN);
	}
}