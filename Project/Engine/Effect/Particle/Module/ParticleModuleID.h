#pragma once

//============================================================================
//	ParticleModuleID
//============================================================================

// 発生処理
enum class ParticleSpawnModuleID {

	Sphere,
	Hemisphere,
	Circle,
	Box,
	Cone,
	PolygonVertex,
	Count,
};

// 更新処理
enum class ParticleUpdateModuleID {

	LifeTime,
	Color,
	Velocity,
	Rotation,
	Scale,
	Gravity,
	Parent,
	ColorUV,
	NoiseUV,
	Emissive,
	AlphaReference,
	Primitive,
	NoiseForce,
	Trail,
	KeyframePath,
	Translate,
	Count,
};