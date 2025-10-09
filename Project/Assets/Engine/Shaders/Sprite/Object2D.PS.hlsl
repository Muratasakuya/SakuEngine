//============================================================================
//	include
//============================================================================

#include "Object2D.hlsli"

//============================================================================
//	Output
//============================================================================

struct PSOutput {

	float4 color : SV_TARGET0;
	uint mask : SV_TARGET1;
};

//============================================================================
//	CBuffer
//============================================================================

cbuffer Material : register(b0) {
	
	float4x4 uvTransform;
	float4 color;
	float3 emissionColor;
	uint useVertexColor;
	uint useAlphaColor;
	float emissiveIntensity;
	float alphaReference;
	uint postProcessMask;
};

//============================================================================
//	Texture Sampler
//============================================================================

Texture2D<float4> gTexture : register(t0);
Texture2D<float4> gAlphaTexture : register(t1);
SamplerState gSampler : register(s0);

//============================================================================
//	Main
//============================================================================
PSOutput main(VSOutput input) {
	
	PSOutput output;
	
	float4 transformUV = mul(float4(input.texcoord, 0.0f, 1.0f), uvTransform);
	
	// alpha値の参照を専用のテクスチャから取得する
	if (useAlphaColor == 1) {
		
		float textureAlpha = gAlphaTexture.Sample(gSampler, transformUV.xy).a;
		// 閾値以下なら破棄
		if (textureAlpha < alphaReference) {
			discard;
		}
	}
	
	float4 textureColor = gTexture.Sample(gSampler, transformUV.xy);
	
	if (textureColor.a <= 0.25f) {
		discard;
	}

	output.color.rgb = color.rgb * textureColor.rgb;
	// α値
	output.color.a = color.a * input.color.a * textureColor.a;
	if (output.color.a <= 0.0f) {
		discard;
	}
	
	//頂点カラー適応
	if (useVertexColor == 1) {

		// rgbのみ
		output.color.rgb *= input.color.rgb;
	}
	
	// emission処理
	// 発光色
	float3 emission = emissionColor * emissiveIntensity;
	// emissionを加算
	output.color.rgb += emission * textureColor.rgb;
	
	// マスク値を出力
	output.mask = postProcessMask;

	return output;
}