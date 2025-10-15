#pragma once

//============================================================================
//	ParticleModuleID
//============================================================================

// 発生処理
enum class ParticleSpawnModuleID {

	Sphere,
	Hemisphere,
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
	UV,
	Emissive,
	AlphaReference,
	Primitive,
	NoiseForce,
	Trail,
	KeyframePath,
	Count,
};