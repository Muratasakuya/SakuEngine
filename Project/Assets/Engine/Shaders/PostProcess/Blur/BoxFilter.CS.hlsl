//============================================================================
//	include
//============================================================================

#include "../PostProcessCommon.hlsli"

//============================================================================
//	Constant
//============================================================================

// 5x5のIndex
static const int2 kIndex5x5[5][5] = {
	{ { -2, -2 }, { -1, -2 }, { 0, -2 }, { 1, -2 }, { 2, -2 } },
	{ { -2, -1 }, { -1, -1 }, { 0, -1 }, { 1, -1 }, { 2, -1 } },
	{ { -2, 0 }, { -1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 } },
	{ { -2, 1 }, { -1, 1 }, { 0, 1 }, { 1, 1 }, { 2, 1 } },
	{ { -2, 2 }, { -1, 2 }, { 0, 2 }, { 1, 2 }, { 2, 2 } }
};

// 5x5カーネル
static const float kKernel5x5[5][5] = {
	{ 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f },
	{ 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f },
	{ 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f },
	{ 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f },
	{ 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f, 1.0f / 25.0f }
};

//============================================================================
//	Main
//============================================================================
[numthreads(THREAD_POSTPROCESS_GROUP, THREAD_POSTPROCESS_GROUP, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint width, height;
	gInputTexture.GetDimensions(width, height);
	
	 // ピクセル位置
	uint2 pixelPos = DTid.xy;

	// 画像範囲外なら処理しない
	if (pixelPos.x >= width || pixelPos.y >= height) {
		return;
	}
	
	// フラグが立っていなければ処理しない
	if (!CheckPixelBitMask(Bit_BoxFilter, pixelPos)) {
		
		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}

	float3 color = float3(0.0f, 0.0f, 0.0f);

	// 5x5 Box Filter 適用
	for (int x = 0; x < 5; ++x) {
		for (int y = 0; y < 5; ++y) {
			
			int2 offset = kIndex5x5[x][y];

			// テクスチャ座標の計算
			int2 samplePos = clamp(pixelPos + offset, int2(0, 0), int2(width - 1, height - 1));
			// サンプリング
			float3 sampleColor = gInputTexture.Load(int3(samplePos, 0)).rgb;
			// カーネル適用
			color += sampleColor * kKernel5x5[x][y];
		}
	}

	gOutputTexture[pixelPos] = float4(color, 1.0f);
}