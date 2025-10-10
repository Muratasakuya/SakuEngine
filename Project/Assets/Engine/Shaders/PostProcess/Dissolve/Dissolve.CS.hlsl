//============================================================================
//	include
//============================================================================

#include "../PostProcessCommon.hlsli"

//============================================================================
//	CBuffer
//============================================================================

struct Material {
	
	float threshold;
	float edgeSize;
	float3 edgeColor;
	float3 thresholdColor;
};

//============================================================================
//	buffer
//============================================================================

Texture2D<float> gDissolveMaskTexture : register(t2);
SamplerState gSampler : register(s0);
ConstantBuffer<Material> gMaterial : register(b0);

//============================================================================
//	Main
//============================================================================
[numthreads(THREAD_POSTPROCESS_GROUP, THREAD_POSTPROCESS_GROUP, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint width, height;
	gInputTexture.GetDimensions(width, height);

	// ピクセル位置
	uint2 pixelPos = DTid.xy;
	
	// 範囲外なら何もしない
	if (pixelPos.x >= width || pixelPos.y >= height) {
		return;
	}
	
	// フラグが立っていなければ処理しない
	if (!CheckPixelBitMask(Bit_Dissolve, gMaskTexture[pixelPos].r)) {
		
		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}
	
	// マスクテクスチャの取得
	float mask = gDissolveMaskTexture.SampleLevel(gSampler, (float2(pixelPos) + 0.5f) / float2(width, height), 0).r;
	
	// 閾値以下なら色をそれ用の色に変更するにする
	if (mask <= gMaterial.threshold) {
		
		gOutputTexture[pixelPos].rgb = gMaterial.thresholdColor;
		gOutputTexture[pixelPos].a = 1.0f;
		return;
	}

	// Edge の計算
	float edge = 1.0f - smoothstep(gMaterial.threshold, gMaterial.threshold + gMaterial.edgeSize, mask);
	// 元のテクスチャカラーを取得
	float4 baseColor = gInputTexture.SampleLevel(gSampler, (float2(pixelPos) + 0.5f) / float2(width, height), 0);
	// Edge を適用
	float3 finalColor = lerp(baseColor.rgb, gMaterial.edgeColor, edge);

	gOutputTexture[pixelPos] = float4(finalColor, baseColor.a);
}