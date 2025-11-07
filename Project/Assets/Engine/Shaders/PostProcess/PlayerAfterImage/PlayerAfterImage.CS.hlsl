//============================================================================
//	include
//============================================================================

#include "../PostProcessCommon.hlsli"

//============================================================================
//	CBuffer
//============================================================================

struct Material {
	
	// 有効フラグ
	int enable;
	
	// ディザリング率(0.0f~1.0f)
	float rate;
	
	// パターン
	int4x4 pattern;
	// ピクセルの割る数
	float divisor;
	
	// ディザリングカラー
	float4 ditherColor;
	
	// 発光
	float3 emissionColor;
	float emissionIntensity;
};

//============================================================================
//	buffer
//============================================================================

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

	// 範囲外
	if (pixelPos.x >= width || pixelPos.y >= height) {
		return;
	}

	// フラグが立っていなければ処理しない
	if (!CheckPixelBitMask(Bit_PlayerAfterImage, pixelPos) || gMaterial.enable == 0) {
	
		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}
	
	// 入力カラー
	float4 color = gInputTexture.Load(int3(pixelPos, 0));
	
	// パターンインデックス
	int2 posIndex = int2(fmod(pixelPos, gMaterial.divisor));
	// ディザリング閾値
	int threshold = gMaterial.pattern[posIndex.y][posIndex.x];
	
	// 閾値以下なら元の色を返す
	if ((threshold - gMaterial.rate) < 0.0f) {

		gOutputTexture[pixelPos] = color;
		return;
	}

	// α値、0.0f~1.0f
	float a = saturate(gMaterial.ditherColor.a);

	// 出力する色をブレンド
	float3 outRGB = gMaterial.ditherColor.rgb * a + color.rgb * (1.0f - a);
	float outA = a + color.a * (1.0f - a);

	// 発光処理
	float3 emissionColor = gMaterial.emissionColor * gMaterial.emissionIntensity;
	outRGB += outRGB * emissionColor;

	// 出力色
	gOutputTexture[pixelPos] = float4(outRGB, outA);
}