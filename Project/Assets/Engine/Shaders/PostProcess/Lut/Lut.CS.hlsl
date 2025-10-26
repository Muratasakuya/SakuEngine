//============================================================================
//	include
//============================================================================

#include "../PostProcessCommon.hlsli"

//============================================================================
//	CBuffer
//============================================================================

cbuffer Params : register(b0) {
	
	float lerpRate;
	float lutSize;
};

//============================================================================
//	buffer
//============================================================================

Texture3D<float4> gLutTexture : register(t2);
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
	if (!CheckPixelBitMask(Bit_Lut, pixelPos)) {
		
		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}
	
	// input取得
	float3 inputColor = gInputTexture.Load(int3(DTid.xy, 0)).rgb;
	// clamp処理しないと帯域外アクセス
	float3 graded = gLutTexture.SampleLevel(gSampler, inputColor, 0.0f).rgb;
	
	// outputに書き込み
	float3 final = lerp(inputColor, graded, lerpRate);
	gOutputTexture[DTid.xy] = float4(final, 1.0f);
}