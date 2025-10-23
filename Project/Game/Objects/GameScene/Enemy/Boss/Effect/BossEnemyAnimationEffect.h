#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/Game/GameEffect.h>

// front
class BossEnemy;
class FollowCamera;

//============================================================================
//	BossEnemyAnimationEffect class
//	アニメーションに合わせたエフェクト制御
//============================================================================
class BossEnemyAnimationEffect {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BossEnemyAnimationEffect() = default;
	~BossEnemyAnimationEffect() = default;

	void Init(const BossEnemy& bossEnemy);

	void Update(BossEnemy& bossEnemy);

	// editor
	void ImGui(const BossEnemy& bossEnemy);

	//--------- accessor -----------------------------------------------------

	void SetFollowCamera(const FollowCamera* followCamera);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- stricture ----------------------------------------------------

	// アニメーションの名前
	enum class AnimationKey {

		None,
		Move,
		LightAttack,
		StrongAttack,
		ChargeAttack,
		ContinuousAttack,
	};

	// 斬撃の種類
	enum class SlashType {

		Light,
		Strong
	};

	// 斬撃
	struct Slash {

		Vector3 translation;
		Vector3 rotation;
		std::unique_ptr<GameEffect> effect;
	};

	// 連続斬撃
	struct ContinuousSlash {

		SlashType slashType;
		float scaling;
		Vector3 translation;
		Vector3 rotation;
	};

	// 発生
	struct Emit {

		bool emitEnble = true;
		Vector3 translation;
		std::unique_ptr<GameEffect> effect;
	};

	//--------- variables ----------------------------------------------------

	const FollowCamera* followCamera_;

	// 現在のアニメーション
	AnimationKey currentAnimationKey_;
	AnimationKey editAnimationKey_;

	// 弱斬撃
	Slash lightSlash_;
	// 強斬撃
	Slash strongSlash_;
	// 連続斬撃
	static const uint32_t continuousCount_ = 3;
	uint32_t continuousEventIndex_; // 現在のキーイベント
	// エフェクトは弱斬撃か強斬撃を使用する
	std::array<ContinuousSlash, continuousCount_> continuousSlashParams_;

	// チャージ
	Emit chargeStar_;   // 星
	Emit chargeCircle_; // 集まってくるエフェクト
	Emit chargeEmit_;   // 攻撃発生

	// 移動時の巻き風
	Emit moveWind_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// update
	void UpdateAnimationKey(BossEnemy& bossEnemy);
	void UpdateEmit(BossEnemy& bossEnemy);
	void UpdateAlways();

	// helper
	void EmitChargeEffect(const BossEnemy& bossEnemy);
	void EmitContinuousEffect(const BossEnemy& bossEnemy, const ContinuousSlash& param);
};