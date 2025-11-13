//============================================================================
//	include
//============================================================================

#include "../../Math/Math.hlsli"
#include "../PostProcessCommon.hlsli"

//============================================================================
//	CBuffer
//============================================================================

struct Material {

	// バイアス
	float bias;
	// 強さ
	float strength;

	float4 color;
	float4x4 uvTransform;
};
ConstantBuffer<Material> gMaterial : register(b0);

//============================================================================
//	Texture
//============================================================================

// 歪みテクスチャ
Texture2D<float4> gDistortionTexture : register(t2);
SamplerState gWrapSampler : register(s0);
SamplerState gClampSampler : register(s1);

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
	if (!CheckPixelBitMask(Bit_DefaultDistortion, pixelPos)) {
		
		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}
	
	// UV座標、サイズで正規化
	float2 pixelUV = (float2(pixelPos) + 0.5f) / float2(width, height);
	
	// 歪みテクスチャのUV
	float2 distortionUV = mul(float4(pixelUV, 0.0f, 1.0f), gMaterial.uvTransform).xy;
	float4 distortionColor = gDistortionTexture.SampleLevel(gWrapSampler, distortionUV, 0.0f);
	
	// 歪みテクスチャでUVにオフセットをかける
	float2 distortionOffset = (distortionColor.rg - gMaterial.bias) * gMaterial.strength;
	float2 distortedUV = pixelUV + distortionOffset;
	
	// シーンテクスチャから最終色を取得
	float4 sceneColor = gInputTexture.SampleLevel(gClampSampler, distortedUV, 0.0f);

	// 最終的な色を出力
	gOutputTexture[pixelPos] = sceneColor;
}