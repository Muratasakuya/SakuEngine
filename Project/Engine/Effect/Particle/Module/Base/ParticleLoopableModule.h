#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Particle/Structures/ParticleLoop.h>
#include <Engine/MathLib/Vector2.h>

//============================================================================
//	ParticleLoopableModule class
//	パーティクルのループ制御が可能なモジュールの基底クラス
//============================================================================
class ParticleLoopableModule {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ParticleLoopableModule() = default;
	virtual ~ParticleLoopableModule() = default;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// ループ制御
	int loopCount_ = 1;           // 回数
	ParticleLoop::Type loopType_; // 種類

	//--------- functions ----------------------------------------------------

	// ループ制御されたT値を計算
	float LoopedT(float rawT) { return ParticleLoop::CalLoopedT(rawT, loopCount_, loopType_); }

	// エディター
	void ImGuiLoopParam();

	// json
	void ToLoopJson(Json& data);
	void FromLoopJson(const Json& data);
};