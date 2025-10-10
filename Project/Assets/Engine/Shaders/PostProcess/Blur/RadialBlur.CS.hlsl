//============================================================================
//	include
//============================================================================

#include "../PostProcessCommon.hlsli"

//============================================================================
//	CBuffer
//============================================================================

struct BlurParameter {
	
	float2 center; // ブラーの中心
	int numSamples; // サンプリング数
	float width; // ぼかしの幅
};
ConstantBuffer<BlurParameter> gBlur : register(b0);

//============================================================================
//	buffer
//============================================================================

SamplerState gSampler : register(s0);

//============================================================================
//	Main
//============================================================================
[numthreads(THREAD_POSTPROCESS_GROUP, THREAD_POSTPROCESS_GROUP, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint width, height;
	gInputTexture.GetDimensions(width, height);

	// ピクセル位置
	uint2 pixelPos = DTid.xy;

	// 範囲外
	if (pixelPos.x >= width || pixelPos.y >= height) {
		return;
	}
	
	// フラグが立っていなければ処理しない
	if (!CheckPixelBitMask(Bit_RadialBlur, gMaskTexture[pixelPos].r)) {
		
		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}
	
	// サンプリング回数が0ならそのままの色で返す
	if (gBlur.numSamples == 0) {

		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}

	// UV座標計算
	float2 uv = (float2(pixelPos) + 0.5f) / float2(width, height);

	// 放射ブラー方向
	float2 direction = normalize(uv - gBlur.center);
	float distance = length(uv - gBlur.center);

	// ブラー適用
	float3 outputColor = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < gBlur.numSamples; ++i) {
		
		float2 sampleUV = uv + direction * gBlur.width * distance * float(i);
		sampleUV = clamp(sampleUV, float2(0.001f, 0.001f), float2(0.999f, 0.999f));
		outputColor += gInputTexture.SampleLevel(gSampler, sampleUV, 0).rgb;
	}

	// 平均化
	outputColor *= rcp(float(gBlur.numSamples));

	gOutputTexture[pixelPos] = float4(outputColor, 1.0f);
}