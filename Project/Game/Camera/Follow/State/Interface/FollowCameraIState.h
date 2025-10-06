#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Base/InputMapper.h>
#include <Engine/Utility/Enum/Direction.h>
#include <Engine/Utility/Timer/StateTimer.h>
#include <Engine/MathLib/MathUtils.h>
#include <Game/Camera/Follow/Structures/FollowCameraStructures.h>
#include <Game/Camera/Follow/Input/FollowCameraInputAction.h>

// front
class FollowCamera;
class Transform3D;

//============================================================================
//	FollowCameraIState class
//============================================================================
class FollowCameraIState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraIState() = default;
	virtual ~FollowCameraIState() = default;

	// 状態遷移時
	virtual void Enter(FollowCamera& followCamera) = 0;

	// 更新処理
	virtual void Update(FollowCamera& followCamera) = 0;

	// 状態終了時
	virtual void Exit() = 0;

	// imgui
	virtual void ImGui(const FollowCamera& followCamera) = 0;

	// json
	virtual void ApplyJson(const Json& data) = 0;
	virtual void SaveJson(Json& data) = 0;

	//--------- accessor -----------------------------------------------------

	void SetInputMapper(const InputMapper<FollowCameraInputAction>* inputMapper) { inputMapper_ = inputMapper; }
	void SetTarget(FollowCameraTargetType type, const Transform3D& target);
	void SetCanExit(bool canExit) { canExit_ = canExit; }

	virtual bool GetCanExit() const { return canExit_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const InputMapper<FollowCameraInputAction>* inputMapper_;
	std::unordered_map<FollowCameraTargetType, const Transform3D*> targets_;

	// 共通parameters
	bool canExit_ = true; // 遷移可能かどうか
};