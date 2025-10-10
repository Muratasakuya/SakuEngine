//============================================================================
//	include
//============================================================================

#include "../PostProcessCommon.hlsli"

//============================================================================
//	Constant
//============================================================================

// 3x3 のオフセット
static const int2 kIndex3x3[3][3] = {
	{ { -1, -1 }, { 0, -1 }, { 1, -1 } },
	{ { -1, 0 }, { 0, 0 }, { 1, 0 } },
	{ { -1, 1 }, { 0, 1 }, { 1, 1 } }
};

// π
static const float PI = 3.14159265f;

// ガウス関数
float Gauss(float x, float y, float sigma) {
	
	float exponent = -(x * x + y * y) / (2.0f * sigma * sigma);
	float denominator = 2.0f * PI * sigma * sigma;
	return exp(exponent) / denominator;
}

//============================================================================
//	CBuffer
//============================================================================

struct GaussParameter {
	
	float sigma; // 標準偏差 ぼかしの強さ
};
ConstantBuffer<GaussParameter> gGauss : register(b0);

//============================================================================
//	Main
//============================================================================
[numthreads(THREAD_POSTPROCESS_GROUP, THREAD_POSTPROCESS_GROUP, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint width, height;
	gInputTexture.GetDimensions(width, height);

	// ピクセル位置
	uint2 pixelPos = DTid.xy;

	// 範囲外ならスキップ
	if (pixelPos.x >= width || pixelPos.y >= height) {
		return;
	}
	
	// フラグが立っていなければ処理しない
	if (!CheckPixelBitMask(Bit_GaussianFilter, gMaskTexture[pixelPos].r)) {

		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}

	// UVステップサイズ
	float2 uvStepSize = float2(1.0f / width, 1.0f / height);

	// カーネルとウェイトの計算
	float kernel3x3[3][3];
	float weight = 0.0f;

	for (int x = 0; x < 3; ++x) {
		for (int y = 0; y < 3; ++y) {
			
			kernel3x3[x][y] = Gauss(kIndex3x3[x][y].x, kIndex3x3[x][y].y, gGauss.sigma);
			weight += kernel3x3[x][y];
		}
	}

	// 畳み込み処理
	float3 blurredColor = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			
			int2 offset = kIndex3x3[i][j];

			// ピクセル位置を計算
			int2 samplePos = clamp(pixelPos + offset, int2(0, 0), int2(width - 1, height - 1));

			// テクスチャのサンプル
			float3 sampleColor = gInputTexture.Load(int3(samplePos, 0)).rgb;

			// カーネル値を掛ける
			blurredColor += sampleColor * kernel3x3[i][j];
		}
	}

	// 正規化
	blurredColor *= rcp(weight);

	gOutputTexture[pixelPos] = float4(blurredColor, 1.0f);
}