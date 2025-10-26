//============================================================================
//	include
//============================================================================

#include "../PostProcessCommon.hlsli"

//============================================================================
//	CBuffer
//============================================================================

struct BlurParam {
	
	int radius;
	float sigma;
};
ConstantBuffer<BlurParam> gBlurParam : register(b0);

//============================================================================
//	Function
//============================================================================

// ガウス関数
float Gaussian(float x, float sigma) {
	
	return exp(-(x * x) / (2.0f * sigma * sigma)) / (sqrt(2.0f * 3.141592653589793) * sigma);
}

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
	if (!CheckPixelBitMask(Bit_VerticalBlur, pixelPos)) {
		
		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}
	
	float4 color = 0;
	float weightSum = 0.0;

	for (int i = -gBlurParam.radius; i <= gBlurParam.radius; i++) {

		int2 offset = int2(0, i);
		// ガウス分布計算
		float weight = Gaussian(float(i), gBlurParam.sigma);

		color += gInputTexture.Load(int3(DTid.xy + offset, 0)) * weight;
		weightSum += weight;
	}

	// 正規化
	gOutputTexture[DTid.xy] = color / weightSum;
}