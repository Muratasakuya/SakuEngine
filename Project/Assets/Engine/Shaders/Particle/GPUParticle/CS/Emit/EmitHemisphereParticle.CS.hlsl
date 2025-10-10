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
struct EmitterHemisphere {
	
	float radius;
	
	float3 translation;
	float4x4 rotationMatrix;
};

ConstantBuffer<EmitterCommon> gEmitterCommon : register(b0);
ConstantBuffer<EmitterHemisphere> gEmitterHemisphere : register(b1);
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

float3 GetRandomDirection(RandomGenerator generator) {
	
	float phi = generator.Generate(0.0f, PI2);
	float z = generator.Generate(-1.0f, 1.0f);
	float sqrtOneMinusZ2 = sqrt(1.0f - z * z);
	float3 direction = float3(sqrtOneMinusZ2 * cos(phi), sqrtOneMinusZ2 * sin(phi), z);
	
	// 半球方向に限定する
	if (direction.y < 0.0f) {
		direction.y -= direction.y;
	}
	
	return normalize(direction);
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
			
		// 球面上へランダムな向きを設定
		float3 direction = GetRandomDirection(generator);
		// 回転を適応
		float3 rotatedDirection = mul(direction, (float3x3) gEmitterHemisphere.rotationMatrix);
		particle.velocity = normalize(rotatedDirection) * generator.Generate3D() * gEmitterCommon.moveSpeed;
			
		Transform transform = (Transform) 0;
		transform.translation = gEmitterHemisphere.translation + direction * gEmitterHemisphere.radius;
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