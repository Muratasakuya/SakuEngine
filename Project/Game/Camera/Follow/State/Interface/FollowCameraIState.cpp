#include "FollowCameraIState.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/Transform.h>
#include <Game/Camera/Follow/FollowCamera.h>

//============================================================================
//	FollowCameraIState classMethods
//============================================================================

void FollowCameraIState::SetTarget(FollowCameraTargetType type,
	const Transform3D& target) {

	targets_[type] = &target;
}