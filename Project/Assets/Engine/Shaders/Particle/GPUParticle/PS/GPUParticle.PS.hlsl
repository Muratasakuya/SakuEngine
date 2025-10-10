//============================================================================
//	include
//============================================================================

#include "../../Common/ParticleCommonSturctures.hlsli"
#include "../../Common/ParticleOutput.hlsli"

//============================================================================
//	Output
//============================================================================

struct PSOutput {
	
	float4 color : SV_TARGET0;
	uint4 mask : SV_TARGET1;
};

//============================================================================
//	StructuredBuffer
//============================================================================

StructuredBuffer<Material> gMaterials : register(t0);

//============================================================================
//	texture Sampler
//============================================================================

Texture2D<float4> gTexture : register(t1);
SamplerState gSampler : register(s0);

//============================================================================
//	Main
//============================================================================
PSOutput main(MSOutput input) {
	
	PSOutput output;
	
	// instanceId、pixelごとの処理
	uint id = input.instanceID;
	Material material = gMaterials[id];
	
	// uv
	float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
	
	// 色
	output.color.rgb = material.color.rgb * textureColor.rgb;
	// α値
	output.color.a = material.color.a * textureColor.a;
	
	// マスク値を出力
	output.mask = uint4(material.postProcessMask, 0, 0, 0);
	
	return output;
}