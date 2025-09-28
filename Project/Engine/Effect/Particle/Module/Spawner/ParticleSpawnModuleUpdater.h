#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/StateTimer.h>
#include <Engine/Utility/Enum/Easing.h>
#include <Engine/MathLib/Vector3.h>

//============================================================================
//	ParticleSpawnModuleUpdater
//============================================================================

// 多角形頂点発生モジュール
class ParticlePolygonVertexUpdater {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	// 値の補間
	void Update(float& scale, Vector3& rotation);

	// editor
	void ImGui();

	// json
	void FromJson(const Json& data);
	void ToJson(Json& data);

	//--------- accessor -----------------------------------------------------

	void SetOffsetRotation(Vector3 offsetRotation);

	float GetStartScale() const { return scale_.start; }
	Vector3 GetStartRotation() const { return rotation_.start; }

	bool CanEmit() const { return timer_.t_ != 0.0f; }
	bool IsFinished() const { return timer_.IsReached(); }
	void Reset() { timer_.Reset(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	template <typename T>
	struct LerpValue {

		T start;
		T target;
		EasingType easing;
	};

	//--------- variables ----------------------------------------------------

	// 時間経過
	StateTimer timer_;

	// スケール
	LerpValue<float> scale_;
	// 回転
	LerpValue<Vector3> rotation_;
};