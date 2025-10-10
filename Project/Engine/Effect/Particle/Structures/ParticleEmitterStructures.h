#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>
#include <Engine/Core/Graphics/PostProcess/PostProcessBit.h>
#include <Engine/MathLib/MathUtils.h>

//============================================================================
//	ParticleEmitterStructures class
//============================================================================

// emitterの形状
enum class ParticleEmitterShape {

	Sphere,
	Hemisphere,
	Box,
	Cone,
	Count,
};

struct ParticleEmitterCommon {

	int32_t count;
	int32_t emit;
	float lifeTime;
	float moveSpeed;

	Vector3 scale;
	float pad1;

	Color color;

	// 適応するポストエフェクトのビット
	uint32_t postProcessMask;
	void Init() {

		// 初期値
		count = 32;
		emit = false;
		lifeTime = 1.0f;
		moveSpeed = 1.0f;

		scale = Vector3::AnyInit(0.4f);
		color = Color::White();

		// デフォルトでかけるポストプロセス
		postProcessMask = Bit_Bloom | Bit_RadialBlur | Bit_Glitch | Bit_Vignette;
	}
};

// 球
struct ParticleEmitterSphere {

	float radius;

	Vector3 translation;
	float pad0;

	void Init() {

		radius = 2.0f;
		translation = Vector3::AnyInit(0.0f);
	}
};

// 半球
struct ParticleEmitterHemisphere {

	float radius;

	Vector3 translation;
	Matrix4x4 rotationMatrix;

	void Init() {

		radius = 2.0f;
		translation = Vector3::AnyInit(0.0f);
		rotationMatrix = Matrix4x4::MakeIdentity4x4();
	}
};

// 箱(OBB)
struct ParticleEmitterBox {

	Vector3 size;
	float pod0;

	Vector3 translation;
	float pod1;

	Matrix4x4 rotationMatrix;

	void Init() {

		size = Vector3::AnyInit(2.0f);
		translation = Vector3::AnyInit(0.0f);
		rotationMatrix = Matrix4x4::MakeIdentity4x4();
	}
};

// コーン状
struct ParticleEmitterCone {

	float baseRadius;
	float topRadius;
	float height;
	float pod0;

	Vector3 translation;
	float pod1;

	Matrix4x4 rotationMatrix;

	void Init() {

		baseRadius = 0.4f;
		topRadius = 1.6f;
		height = 1.6f;

		translation = Vector3::AnyInit(0.0f);
		rotationMatrix = Matrix4x4::MakeIdentity4x4();
	}
};

struct ParticleEmitterData {

	ParticleEmitterShape shape;

	// 発生
	ParticleEmitterCommon common;

	// 球
	ParticleEmitterSphere sphere;
	// 半球
	ParticleEmitterHemisphere hemisphere;
	// 箱(OBB)
	ParticleEmitterBox box;
	// コーン状
	ParticleEmitterCone cone;

	void Init() {

		// 全て初期化
		sphere.Init();
		hemisphere.Init();
		box.Init();
		cone.Init();
	}
};

struct ParticleEmitterBufferData {

	// 発生
	DxConstBuffer<ParticleEmitterCommon> common;

	// 球
	DxConstBuffer<ParticleEmitterSphere> sphere;
	// 半球
	DxConstBuffer<ParticleEmitterHemisphere> hemisphere;
	// 箱(OBB)
	DxConstBuffer<ParticleEmitterBox> box;
	// コーン状
	DxConstBuffer<ParticleEmitterCone> cone;
};