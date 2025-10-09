//============================================================================
//	include
//============================================================================

#include "MeshStandard.hlsli"
#include "Dither.hlsli"

//============================================================================
//	structure
//============================================================================

struct Material {

	float4 color;
	
	uint textureIndex;
	uint normalMapTextureIndex;
	int enableNormalMap;
	int enableDithering;
	
	float emissiveIntensity;
	float3 emissionColor;
		
	float4x4 uvTransform;

	uint postProcessMask;
};

struct Lighting {
	
	int enableLighting;
	int enableHalfLambert;
	int enableBlinnPhongReflection;
	int enableImageBasedLighting;
	int castShadow;
	
	float shininess;
	float3 specularColor;
	
	float shadowRate;
	float environmentCoefficient;
};

//============================================================================
//	buffer
//============================================================================

cbuffer PunctualLight : register(b0) {
	
	DirectionalLight directionalLight;
	PointLight pointLight;
	SpotLight spotLight;
};

cbuffer Camera : register(b1) {

	float3 worldPosition;
};

cbuffer Scene : register(b2) {

	// ray
	float rayMin;
	float rayMax;
};

RaytracingAccelerationStructure gScene : register(t0);
StructuredBuffer<Material> gMaterials : register(t1);
StructuredBuffer<Lighting> gLightings : register(t2);

//============================================================================
//	texture Sampler
//============================================================================

Texture2D<float4> gTextures[] : register(t3, space0);
TextureCube<float4> gEnvironmentTexture : register(t4, space1);

SamplerState gSampler : register(s0);

//============================================================================
//	functions
//============================================================================

float4 GetDiffuseColor(Material material, float4 transformUV, MSOutput input) {
	
	// texture
	float4 diffuseColor = gTextures[material.textureIndex].Sample(gSampler, transformUV.xy);
	
	return diffuseColor;
}

float3 GetNormalMap(Material material, float4 transformUV, MSOutput input) {
	
	// normalMap
	float3 localNormal = gTextures[material.normalMapTextureIndex].Sample(gSampler, transformUV.xy).xyz;
	localNormal = (localNormal - 0.5f) * 2.0f;
	float3 normal = input.tangent * localNormal.x + input.biNormal * localNormal.y + input.normal * localNormal.z;
	
	return normal;
}

float3 CalculateLambertLighting(Material material, float3 normal, float3 diffuseColor) {
	
	float NdotL = dot(normalize(normal), normalize(-directionalLight.direction));
	float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
	return material.color.rgb * diffuseColor.rgb * directionalLight.color.rgb * cos * directionalLight.intensity;
}

float3 CalculateBlinnPhongLighting(Material material, Lighting lighting, float3 normal, float3 diffuseColor, MSOutput input) {
	
	float3 result = float3(0.0f, 0.0f, 0.0f);

	float3 pointLightDirection = normalize(input.worldPosition - pointLight.pos);
	float distancePointLight = length(pointLight.pos - input.worldPosition);
	float factorPointLight = pow(saturate(-distancePointLight / pointLight.radius + 1.0f), pointLight.decay);

	float3 spotLightDirection = normalize(input.worldPosition - spotLight.pos);
	float distanceSpotLight = length(spotLight.pos - input.worldPosition);
	float factorSpotLight = pow(saturate(-distanceSpotLight / spotLight.distance + 1.0f), spotLight.decay);
	float cosAngle = dot(spotLightDirection, spotLight.direction);
	float falloffFactor = saturate((cosAngle - spotLight.cosAngle) / (spotLight.cosFalloffStart - spotLight.cosAngle));

	float3 toEye = normalize(worldPosition - input.worldPosition);
	
	float3 halfPL = normalize(-pointLightDirection + toEye);
	float specPL = pow(saturate(dot(normalize(normal), halfPL)), lighting.shininess);
	float cosPL = pow(dot(normalize(normal), -pointLightDirection) * 0.5f + 0.5f, 2.0f);

	float3 halfSL = normalize(-spotLightDirection + toEye);
	float specSL = pow(saturate(dot(normalize(normal), halfSL)), lighting.shininess);
	float cosSL = pow(dot(normalize(normal), -spotLightDirection) * 0.5f + 0.5f, 2.0f);

	float3 diffusePL = material.color.rgb * diffuseColor.rgb * pointLight.color.rgb * cosPL * pointLight.intensity * factorPointLight;
	float3 specularPL = pointLight.color.rgb * pointLight.intensity * factorPointLight * specPL * lighting.specularColor;

	float3 diffuseSL = material.color.rgb * diffuseColor.rgb * spotLight.color.rgb * cosSL * spotLight.intensity * falloffFactor * factorSpotLight;
	float3 specularSL = spotLight.color.rgb * spotLight.intensity * falloffFactor * factorSpotLight * specSL * lighting.specularColor;

	result += diffusePL + specularPL + diffuseSL + specularSL;
	return result;
}

float3 CalculateImageBasedLighting(float3 normal, float environmentCoefficient, MSOutput input) {
	
	float3 cameraToPos = normalize(input.worldPosition - worldPosition);
	float3 reflectedVector = reflect(cameraToPos, normalize(normal));
	float4 environmentColor = gEnvironmentTexture.Sample(gSampler, reflectedVector);

	return environmentColor.rgb * environmentCoefficient;
}

bool IsShadowed(float3 origin) {
	
	// rayDesc
	RayDesc rayDesc;
	rayDesc.Origin = origin;
	rayDesc.Direction = -directionalLight.direction;
	rayDesc.TMin = rayMin;
	rayDesc.TMax = rayMax;
		
	// createRayQueryObject
	RayQuery < 0 > rayQuery;
	// executeRay
	rayQuery.TraceRayInline(gScene, 0, 0xFE, rayDesc);
	
	while (rayQuery.Proceed()) {
	}

	return rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT;
}