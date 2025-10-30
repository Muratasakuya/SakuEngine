#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/MathUtils.h>

//============================================================================
//	ParticleShape
//============================================================================

// 形状
enum class ParticlePrimitiveType {

	Plane,     // 平面
	Ring,      // リング
	Cylinder,  // 円柱
	Crescent,  // 三日月
	Lightning, // 雷
	Count
};

// 平面の種類
enum class ParticlePlaneType {

	XY,
	XZ
};

// 平面
struct PlaneForGPU {

	Vector2 size;
	Vector2 pivot;

	// 0: XY
	// 1: XZ
	uint32_t mode;

	void Init() {

		size = Vector2::AnyInit(1.0f);
		pivot = Vector2::AnyInit(0.5f);

		mode = 0;
	}
};
// リング
struct RingForGPU {

	float outerRadius;
	float innerRadius;
	int divide;

	void Init() {

		outerRadius = 4.0f;
		innerRadius = 2.0f;
		divide = 8;
	}
};
// 円柱
struct CylinderForGPU {

	float topRadius;
	float bottomRadius;
	float height;
	int divide;

	void Init() {

		topRadius = 4.0f;
		bottomRadius = 4.0f;
		height = 4.0f;
		divide = 16;
	}
};
// 三日月
struct CrescentForGPU {

	// 半径
	float outerRadius;
	float innerRadius;

	// 始点と終点
	float startAngle;
	float endAngle;

	// 変形
	float lattice;
	float thickness;
	Vector2 pivot;

	// 頂点色
	Color outerColor;
	Color innerColor;

	int divide;

	// 0: 縦
	// 1: 横
	int uvMode;

	void Init() {

		outerRadius = 4.0f;
		innerRadius = 2.0f;

		// 30度~150度
		startAngle = pi / 6.0f;
		endAngle = pi * 5.0f / 6.0f;

		lattice = 0.5f;
		thickness = 0.1f;
		pivot = Vector2::AnyInit(0.5f);

		outerColor = Color::White();
		innerColor = Color::White();

		divide = 8;
		uvMode = 1;
	}
};

// 雷
struct LightningForGPU {

	Vector3 start; // 開始位置
	Vector3 end;   // 終了位置

	float width;        // 幅
	uint32_t nodeCount; // ノード数

	float amplitudeRatio; // 振幅倍率
	float frequency;      // うねり回数
	float smoothness;     // 0.0f~1.0f、大きいほど滑らかになる
	float time;           // 更新時間

	float angle; // 雷の孤の角度、-180~180度

	float seed; // 乱数シード値

	void Init() {

		start = Vector3(0.0f, 32.0f, 0.0f);
		end = Vector3(0.0f, 1.0f, 0.0f);

		width = 2.0f;
		nodeCount = 20;

		amplitudeRatio = 0.48f;
		frequency = 6.0f;
		smoothness = 0.32f;
		time = 0.0f;

		angle = 0.0f;

		seed = 1337.0f;
	}
};