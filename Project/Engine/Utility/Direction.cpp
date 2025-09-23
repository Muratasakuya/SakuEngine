#include "Direction.h"

//============================================================================
//	include
//============================================================================
#include <Lib/MathUtils/MathUtils.h>

//============================================================================
//	Direction classMethods
//============================================================================

Vector3 Direction::Get(Direction3D direction) {

	Vector3 result{};
	switch (direction) {
	case Direction3D::Forward:

		result = Vector3(0.0f, 0.0f, 1.0f);
		break;
	case Direction3D::Backward:

		result = Vector3(0.0f, 0.0f, -1.0f);
		break;
	case Direction3D::Right:

		result = Vector3(1.0f, 0.0f, 0.0f);
		break;
	case Direction3D::Left:

		result = Vector3(-1.0f, 0.0f, 0.0f);
		break;
	case Direction3D::Up:

		result = Vector3(0.0f, 1.0f, 0.0f);
		break;
	case Direction3D::Bottom:

		result = Vector3(0.0f, -1.0f, 0.0f);
		break;
	}
	return result;
}

Vector3 Direction::GetRotate(Direction3D direction) {

	Vector3 result{};
	switch (direction) {
	case Direction3D::Forward:

		result = Vector3(0.0f, 0.0f, 0.0f);
		break;
	case Direction3D::Backward:

		result = Vector3(0.0f, pi, 0.0f);
		break;
	case Direction3D::Right:

		result = Vector3(0.0f, pi / 2.0f, 0.0f);
		break;
	case Direction3D::Left:

		result = Vector3(0.0f, -pi / 2.0f, 0.0f);
		break;
	case Direction3D::Up:

		result = Vector3(-pi / 2.0f, 0.0f, 0.0f);
		break;
	case Direction3D::Bottom:

		result = Vector3(pi / 2.0f, 0.0f, 0.0f);
		break;
	}
	return result;
}

Vector2 Direction::Get(Direction2D direction) {

	Vector2 result{};
	switch (direction) {
	case Direction2D::Right:

		result = Vector2(1.0f, 0.0f);
		break;
	case Direction2D::Left:

		result = Vector2(-1.0f, 0.0f);
		break;
	case Direction2D::Up:

		result = Vector2(0.0f, 1.0f);
		break;
	case Direction2D::Bottom:

		result = Vector2(0.0f, -1.0f);
		break;
	}
	return result;
}