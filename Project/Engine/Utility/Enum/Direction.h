#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector2.h>
#include <Engine/MathLib/Vector3.h>

//============================================================================
//	Direction
//============================================================================

// 3次元
enum class Direction3D {

	Forward,  // +Z
	Backward, // -Z
	Right,    // +X
	Left,     // -X
	Up,       // +Y
	Bottom    // -Y
};
// 2次元
enum class Direction2D {

	Right, // +X
	Left,  // -X
	Up,    // +Y
	Bottom // -Y
};

namespace Direction {

	// 3D
	Vector3 Get(Direction3D direction);
	Vector3 GetRotate(Direction3D direction);

	// 2D
	Vector2 Get(Direction2D direction);
}