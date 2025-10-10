//============================================================================
//	include
//============================================================================

#include "../../../Common/ParticleCommonSturctures.hlsli"
#include "../../ParticleEmitterStructures.hlsli"
#include "../../../../../../../Engine/Effect/Particle/ParticleConfig.h"

#include "../../../../Math/Math.hlsli"

//============================================================================
//	CBuffer
//============================================================================

// 形状
struct EmitterCone {
	
	float baseRadius;
	float topRadius;
	float height;
	
	float3 translation;
	float4x4 rotationMatrix;
};

ConstantBuffer<EmitterCommon> gEmitterCommon : register(b0);
ConstantBuffer<EmitterCone> gEmitterCone : register(b1);
ConstantBuffer<PerFrame> gPerFrame : register(b2);

//============================================================================
//	RWStructuredBuffer
//============================================================================

RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<Transform> gTransform : register(u1);
RWStructuredBuffer<Material> gMaterials : register(u2);

RWStructuredBuffer<int> gFreeListIndex : register(u3);
RWStructuredBuffer<uint> gFreeList : register(u4);

//============================================================================
//	Functions
//============================================================================

float3 GetFacePoint(RandomGenerator generator, float radius, float height) {
	
	float angle = generator.Generate(0.0f, PI2);
	float radiusRadom = generator.Generate(0.0f, radius);

	return float3(radiusRadom * cos(angle), height, radiusRadom * sin(angle));
}

//============================================================================
//	Main
//============================================================================
[numthreads(THREAD_EMIT_GROUP, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	// 発生許可が下りていなければ処理しない
	if (gEmitterCommon.emit == 0 ||
		gEmitterCommon.count <= DTid.x) {
		return;
	}
	
	// 乱数生成
	RandomGenerator generator;
	generator.seed = (DTid + gPerFrame.time) * gPerFrame.time;

	int freeListIndex;
	InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
		
	if (0 <= freeListIndex && freeListIndex < kMaxGPUParticles) {
			
		uint particleIndex = gFreeList[freeListIndex];
			
		Particle particle;
		particle.currentTime = 0.0f;
		particle.lifeTime = gEmitterCommon.lifeTime;
			
		// 向いている方向(+Z)方向に飛ばす
		// 上の面と下の面の座標取得
		// ローカル
		float3 baseLocal = GetFacePoint(generator, gEmitterCone.baseRadius, 0.0f);
		float3 topLocal = GetFacePoint(generator, gEmitterCone.topRadius, gEmitterCone.height);
		// ワールド
		float3 baseWorld = gEmitterCone.translation + mul(float4(baseLocal, 1.0f), gEmitterCone.rotationMatrix).xyz;
		float3 topWorld = gEmitterCone.translation + mul(float4(topLocal, 1.0f), gEmitterCone.rotationMatrix).xyz;
		float3 direction = normalize(topWorld - baseWorld);
		particle.velocity = direction * generator.Generate3D() * gEmitterCommon.moveSpeed;
			
		Transform transform = (Transform) 0;
		transform.translation = baseWorld;
		transform.scale = gEmitterCommon.scale;
			
		Material material = (Material) 0;
		material.color = gEmitterCommon.color;
		material.postProcessMask = gEmitterCommon.postProcessMask;

		// 値を設定
		gParticles[particleIndex] = particle;
		gTransform[particleIndex] = transform;
		gMaterials[particleIndex] = material;
	} else {

		InterlockedAdd(gFreeListIndex[0], 1);
	}
}