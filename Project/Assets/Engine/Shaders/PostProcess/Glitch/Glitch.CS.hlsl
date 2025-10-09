//============================================================================
//	include
//============================================================================

#include "../../Math/Math.hlsli"
#include "../PostProcessCommon.hlsli"

//============================================================================
//	CBuffer
//============================================================================

struct GlitchMaterial {
	
	float time; // 経過秒
	float intensity; // グリッチ全体の強さ
	float blockSize; // 横ずれブロックの太さ
	float colorOffset; // RGBずれ距離
	float noiseStrength; // ノイズ混合率
	float lineSpeed; // スキャンライン走査速度
};
ConstantBuffer<GlitchMaterial> gMaterial : register(b0);

//============================================================================
//	Texture
//============================================================================

Texture2D<float> gNoiseTexture : register(t2);
SamplerState gSampler : register(s0);

//============================================================================
//	main
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
	if (!CheckPixelBitMask(Bit_Glitch, gMaskTexture[pixelPos])) {
		
		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}
	
	float2 uv = (float2) DTid.xy / float2(width, height);
	
	// 横スキャンラインずらし
	float linePhase = Hash12(float2(0.0f, DTid.y * 7.13f) + gMaterial.time * gMaterial.lineSpeed);
	// たまに大きく跳ねるブロックグリッチ
	float lineAmp = smoothstep(0.8f, 1.0f, linePhase);
	// ブロック単位で段差状にしたい場合
	float blockStep = round(DTid.x / gMaterial.blockSize) * gMaterial.blockSize;
	float2 glitchOffs = float2(lineAmp * gMaterial.intensity * gMaterial.blockSize, 0);

	// 元ピクセルとずらしたピクセル取得
	int2 srcPos = int2(clamp(int(DTid.x + glitchOffs.x), 0, int(width - 1)),DTid.y);
	float4 srcColor = gInputTexture.Load(int3(srcPos, 0));
	
	// Chromatic Aberration風
	float2 rgbShift = float2((Hash12(float2(gMaterial.time, srcPos.y)) - 0.5f) * 2.0f,
	(Hash12(float2(gMaterial.time + 11.3f, srcPos.y)) - 0.5f) * 2.0f) *
	gMaterial.colorOffset * gMaterial.intensity;

	// r g,bそれぞれ別座標からサンプル
	int2 offR = clamp(srcPos + int2(rgbShift), int2(0, 0), int2(width - 1, height - 1));
	int2 offB = clamp(srcPos + int2(-rgbShift), int2(0, 0), int2(width - 1, height - 1));

	float3 col;
	col.r = gInputTexture.Load(int3(offR, 0)).r;
	col.g = srcColor.g;
	col.b = gInputTexture.Load(int3(offB, 0)).b;

	// ホワイトノイズオーバーレイ
	float noise = gNoiseTexture.SampleLevel(gSampler, uv * 512.0f, 0.0f).r;
	col = lerp(col, float3(noise, noise, noise), gMaterial.noiseStrength * gMaterial.intensity);

	// 最終的な色を出力
	gOutputTexture[DTid.xy] = float4(col, 1.0f);
}