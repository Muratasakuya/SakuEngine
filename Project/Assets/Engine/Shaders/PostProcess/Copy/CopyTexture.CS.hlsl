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

	// テクスチャのサンプル
	float4 color = gInputTexture.Load(int3(pixelPos, 0));
	// α値を1.0fに固定
	gOutputTexture[pixelPos] = float4(color.rgb, 1.0f);
}