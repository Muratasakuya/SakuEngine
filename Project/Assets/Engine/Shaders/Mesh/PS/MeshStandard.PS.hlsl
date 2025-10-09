//============================================================================
//	include
//============================================================================

#include "../StandardMSFunction.hlsli"

//============================================================================
//	Output
//============================================================================

struct PSOutput {
	
	float4 color : SV_TARGET0;
	uint mask : SV_TARGET1;
};

//============================================================================
//	Main
//============================================================================
PSOutput main(MSOutput input) {
	
	PSOutput output;
	
	// instanceId、Pixelごとの処理
	uint id = input.instanceID;
	
	// bufferアクセス
	Material material = gMaterials[id];
	Lighting lighting = gLightings[id];
	
	// ディザ抜き処理を行うか
	if (material.enableDithering == 1) {
		
		// pixelの距離を計算
		float distanceToEye = distance(worldPosition, input.worldPosition);
		ApplyDistanceDither(input.position.xy, distanceToEye);
	}

	// uv
	float4 transformUV = mul(float4(input.texcoord, 0.0f, 1.0f), material.uvTransform);
	// diffuseColor
	float4 diffuseColor = GetDiffuseColor(material, transformUV, input);
	// α値
	output.color.a = material.color.a * diffuseColor.a;
	
	// normal
	float3 normal = input.normal;
	if (material.enableNormalMap == 1) {
		
		// normalMapTexture
		normal = GetNormalMap(material, transformUV, input);
	}
	
	// lighting
	if (lighting.enableLighting == 1) {
		if (lighting.enableHalfLambert == 1) {
			
			// halfLambert処理
			output.color.rgb = CalculateLambertLighting(material, normal, diffuseColor.rgb);
		}
		if (lighting.enableBlinnPhongReflection == 1) {
			
			// blinnPhong処理
			output.color.rgb += CalculateBlinnPhongLighting(material, lighting, normal, diffuseColor.rgb, input);
		}
	} else {

		// lighting無効
		output.color.rgb = material.color.rgb * diffuseColor.rgb;
	}
	if (lighting.enableImageBasedLighting == 1) {
	
		// imageBasedLighting処理
		output.color.rgb += CalculateImageBasedLighting(normal, lighting.environmentCoefficient, input);
	}
	
	// emissive
	float3 emission = material.emissionColor * material.emissiveIntensity;
	output.color.rgb += emission * diffuseColor.rgb;
	
	if (lighting.shadowRate < 1.0f) {
		
		// 影判定
		bool isShadowed = IsShadowed(input.worldPosition);
		// 影の時
		if (isShadowed) {
		
			// 薄い黒
			output.color.rgb *= lighting.shadowRate;
		}
	}
	
	// マスク値を出力
	output.mask = material.postProcessMask;
	
	return output;
}