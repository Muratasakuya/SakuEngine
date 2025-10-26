//============================================================================
//	include
//============================================================================

#include "../PostProcessCommon.hlsli"

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
	if (!CheckPixelBitMask(Bit_SepiaTone, pixelPos)) {
		
		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}

	// テクスチャのサンプル
	float4 color = gInputTexture.Load(int3(pixelPos, 0));
	// グレースケールの計算
	float luminance = dot(color.rgb, float3(0.2125f, 0.7154f, 0.0721f));
	// セピアトーン適用
	float3 sepiaColor = luminance * float3(1.0f, 74.0f / 107.0f, 43.0f / 107.0f);

	gOutputTexture[pixelPos] = float4(sepiaColor, color.a);
}