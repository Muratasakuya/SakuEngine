//============================================================================
//	include
//============================================================================

#include "../../Math/Math.hlsli"
#include "../PostProcessCommon.hlsli"

//============================================================================
//	CBuffer
//============================================================================

struct RandomMaterial {
	
	float time;
	uint enable;
};
ConstantBuffer<RandomMaterial> gMaterial : register(b0);

//============================================================================
//	main
//============================================================================
[numthreads(THREAD_POSTPROCESS_GROUP, THREAD_POSTPROCESS_GROUP, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint width, height;
	gInputTexture.GetDimensions(width, height);
	
	 // ピクセル位置
	uint2 pixelPos = DTid.xy;

	// 画像範囲外チェック
	if (pixelPos.x >= width || pixelPos.y >= height) {
		return;
	}
	
	// フラグが立っていなければ処理しない
	if (!CheckPixelBitMask(Bit_Random, gMaskTexture[pixelPos].r)) {
		
		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}
	
	// 有効でないなら入力画像の色をそのまま返す
	if (gMaterial.enable == 0) {
		
		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}
	
	// 乱数、時間で変化させる
	float random = Rand2DTo1D(pixelPos * gMaterial.time);
	gOutputTexture[pixelPos] = float4(random, random, random, 1.0f);
}