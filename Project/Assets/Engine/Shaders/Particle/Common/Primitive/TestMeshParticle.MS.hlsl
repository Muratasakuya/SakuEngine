//============================================================================
//	include
//============================================================================

#include "../ParticleOutput.hlsli"
#include "../ParticleCommonSturctures.hlsli"

#include "../../../Math/Math.hlsli"

//============================================================================
//	CBuffer
//============================================================================

ConstantBuffer<PerView> gPerView : register(b0);

//============================================================================
//	StructuredBuffer
//============================================================================

struct TestMesh {
	
	float2 size;
	float2 pivot;
	uint mode;
};

StructuredBuffer<TestMesh> gMeshes : register(t0);
StructuredBuffer<Transform> gTransform : register(t1);

//============================================================================
//	Main
//============================================================================
[numthreads(4, 1, 1)]
[outputtopology("triangle")]
void main(uint groupThreadId : SV_GroupThreadID, uint groupId : SV_GroupID,
out vertices MSOutput verts[4], out indices uint3 polys[2]) {
	
	// dispatchMeshでの1次元グループID
	uint instanceIndex = groupId;
	// バッファアクセス
	TestMesh mesh = gMeshes[instanceIndex];
	Transform transform = gTransform[instanceIndex];
	
	// 頂点数4,出力三角形数2
	SetMeshOutputCounts(4, 2);
	
	// 三角形インデックスは最初の2スレッドで書き込み
	if (groupThreadId < 2) {

		const uint3 index[2] = { uint3(0, 1, 2), uint3(2, 1, 3) };
		polys[groupThreadId] = index[groupThreadId];
	}
	
	// planeを生成
	if (groupThreadId < 4) {
		
		// 縦と横の半分のサイズ
		float2 halfSize = mesh.size * 0.5f;

		float3 localPos;
		float2 uv = float2(0.0f, 0.0f);
		switch (groupThreadId) {
			// 下左
			case 0:

				localPos = float3(-halfSize.x, -halfSize.y, 0.0f);
				uv = float2(0.0f, 1.0f);
				break;
			// 下右
			case 1:

				localPos = float3(halfSize.x, -halfSize.y, 0.0f);
				uv = float2(1.0f, 1.0f);
				break;
			// 上左
			case 2:

				localPos = float3(-halfSize.x, halfSize.y, 0.0f);
				uv = float2(0.0f, 0.0f);
				break;
			// 上右
			default:

				localPos = float3(halfSize.x, halfSize.y, 0.0f);
				uv = float2(1.0f, 0.0f);
				break;
		}
		
		// ピボットを適応
		float2 pivotOffset = lerp(-halfSize, halfSize, mesh.pivot);
		localPos.xy -= pivotOffset;
		
		// XY平面
		if (mesh.mode == 0) {

			localPos = float3(localPos.x, localPos.y, 0.0f);
		}
		// XZ平面
		else if (mesh.mode == 1) {

			localPos = float3(localPos.x, 0.0f, localPos.y);
		}
		
		// world行列を作成
		float4x4 worldMatrix = MakeWorldMatrix(transform, gPerView.billboardMatrix, gPerView.cameraPos);
		worldMatrix[3].xyz = transform.translation;
		
		// 親の設定
		worldMatrix = SetParent(transform, worldMatrix);

		// 行列計算
		float4x4 wvp = mul(worldMatrix, gPerView.viewProjection);
		float4 pos = mul(float4(localPos, 1.0f), wvp);

		// 頂点情報を設定
		MSOutput vertex;
		vertex.position = pos;
		vertex.texcoord = uv;
		vertex.instanceID = instanceIndex;
		vertex.vertexColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
		
		verts[groupThreadId] = vertex;
	}
}