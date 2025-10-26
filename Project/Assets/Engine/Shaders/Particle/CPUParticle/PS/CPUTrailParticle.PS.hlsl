//============================================================================
//	include
//============================================================================

#include "../../Common/ParticleOutput.hlsli"

//============================================================================
//	Output
//============================================================================

struct PSOutput {
	
	float4 color : SV_TARGET0;
	uint mask : SV_TARGET1;
};

//============================================================================
//	StructuredBuffer
//============================================================================

struct Material {
	
	float4 color;

	// 発光
	float emissiveIntensity;
	float3 emissionColor;

	// 閾値
	float alphaReference;
	float noiseAlphaReference;

	float4x4 uvTransform;
	
	uint postProcessMask;
};

struct TextureInfo {
	
	// texture
	uint colorTextureIndex;
	uint noiseTextureIndex;
	
	// sampler
	// 0...WRAP
	// 1...CLAMP
	uint samplerType;
	
	// flags
	int useNoiseTexture;
};

StructuredBuffer<Material> gMaterials : register(t0);
StructuredBuffer<TextureInfo> gTextueInfos : register(t1);

//============================================================================
//	texture Sampler
//============================================================================

Texture2D<float4> gTextures[] : register(t2, space0);
SamplerState gWRAPSampler : register(s0);
SamplerState gCLAMPSampler : register(s1);

//============================================================================
//	Functions
//============================================================================

void CheckDiscard(float alpha, float reference) {
	
	// 閾値以下なら棄却
	if (alpha < reference) {
		discard;
	}
}

float4 GetTextureColor(TextureInfo textureInfo, float4 transformUV) {
	
	float4 textureColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	// サンプラーの種類で分岐
	if (textureInfo.samplerType == 0) {
		
		textureColor = gTextures[textureInfo.colorTextureIndex].Sample(gWRAPSampler, transformUV.xy);
	} else if (textureInfo.samplerType == 1) {
		
		textureColor = gTextures[textureInfo.colorTextureIndex].Sample(gCLAMPSampler, transformUV.xy);
	}
	return textureColor;
}

void CheckNoiseDiscard(Material material, TextureInfo textureInfo, float4 transformUV) {
	
	// ノイズを使用しないならそもそも処理しない
	if (textureInfo.useNoiseTexture == 0) {
		return;
	}
	
	float4 color = gTextures[textureInfo.noiseTextureIndex].Sample(gWRAPSampler, transformUV.xy).r;
	
	// 閾値以下なら棄却
	CheckDiscard(color.a, material.noiseAlphaReference);
}

void ApplyEmissive(inout float3 color, float3 textureRGB, Material material) {
	
	// 発光色
	float3 emission = material.emissionColor * material.emissiveIntensity;
	color += emission * textureRGB;
}

//============================================================================
//	Main
//============================================================================
PSOutput main(MSOutput input) {
		
	PSOutput output;
	
	// instanceId、pixelごとの処理
	uint id = input.instanceID;
	Material material = gMaterials[id];
	TextureInfo textureInfo = gTextueInfos[id];
	
	// uv変形
	float4 transformUV = mul(float4(input.texcoord, 0.0f, 1.0f), material.uvTransform);

	// ノイズ棄却判定
	CheckNoiseDiscard(material, textureInfo, transformUV);

	// テクスチャ色取得、棄却判定
	float4 textureColor = GetTextureColor(textureInfo, transformUV);
	CheckDiscard(textureColor.a, material.alphaReference);
	
	// 色
	output.color.rgb = material.color.rgb * textureColor.rgb * input.vertexColor.rgb;
	
	// 発光処理
	ApplyEmissive(output.color.rgb, textureColor.rgb, material);
	
	// α値
	output.color.a = material.color.a * textureColor.a * input.vertexColor.a;

	// マスク値を出力
	output.mask = material.postProcessMask;
	
	return output;
}