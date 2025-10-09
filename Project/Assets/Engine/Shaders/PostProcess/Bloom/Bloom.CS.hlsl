//============================================================================
//	include
//============================================================================

#include "../PostProcessCommon.hlsli"

//============================================================================
//	CBuffer
//============================================================================

cbuffer Parameter : register(b0) {
	
	float threshold;
	int radius;
	float sigma;
};

//============================================================================
//	Function
//============================================================================

// ガウス関数
float Gaussian(float x, float s) {
	
	return exp(-(x * x) / (2.0f * s * s));
}

//============================================================================
//	Main
//============================================================================
[numthreads(THREAD_POSTPROCESS_GROUP, THREAD_POSTPROCESS_GROUP, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint width, height;
	gInputTexture.GetDimensions(width, height);

	// 現在処理中のピクセル
	int2 pixelPos = int2(DTid.xy);

	// 元カラー保持
	float4 sceneColor = gInputTexture.Load(int3(pixelPos, 0));
	// 範囲外
	if (pixelPos.x >= width || pixelPos.y >= height) {
		return;
	}
	
	// フラグが立っていなければ処理しない
	if (!CheckPixelBitMask(Bit_Bloom, gMaskTexture[pixelPos])) {

		gOutputTexture[pixelPos] = sceneColor;
		return;
	}

	// サンプリング処理
	float3 bloomAccum = 0.0f;
	float weightSum = 0.0f;
	
	for (int y = -radius; y <= radius; ++y) {
		
		float wy = Gaussian((float) y, sigma);
		for (int x = -radius; x <= radius; ++x) {
			
			float wx = Gaussian((float) x, sigma);
			float w = wx * wy;

			int2 samplePos = pixelPos + int2(x, y);
			samplePos = clamp(samplePos, int2(0, 0), int2(int(width) - 1, int(height) - 1));

			float4 color = gInputTexture.Load(int3(samplePos, 0));

			// 輝度抽出
			float luminance = dot(color.rgb, float3(0.2125f, 0.7154f, 0.0721f));
			if (luminance < threshold) {
				color = float4(0.0f, 0.0f, 0.0f, 0.0f);
			}

			bloomAccum += color.rgb * w;
			weightSum += w;
		}
	}
	
	// 合成処理
	float3 bloomColor = (weightSum > 0.0f) ? bloomAccum / weightSum : 0.0f;
	float3 finalColor = sceneColor.rgb + bloomColor;
	gOutputTexture[pixelPos] = float4(finalColor, 1.0f);
}