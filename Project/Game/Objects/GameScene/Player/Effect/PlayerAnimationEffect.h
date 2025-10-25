#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Effect/User/GameEffect.h>

// front
class Player;

//============================================================================
//	PlayerAnimationEffect class
//	アニメーションに合わせたエフェクトの管理
//============================================================================
class PlayerAnimationEffect {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerAnimationEffect() = default;
	~PlayerAnimationEffect() = default;

	// 初期化
	void Init(const Player& player);

	// エフェクトの発生、更新
	void Update(Player& player);

	// editor
	void ImGui(const Player& player);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- stricture ----------------------------------------------------

	// アニメーションの名前
	enum class AnimationKey {

		None,
		Attack_1st,
		Attack_2nd,
		Attack_3rd,
		Attack_4th,
		Skil
	};

	// 斬撃
	struct Slash {

		float scaling;
		Vector3 translation;
		Vector3 rotation;
	};

	// 剣
	struct Sword {

		std::unique_ptr<GameEffect> slashEffect;
		std::unique_ptr<GameEffect> sparkEffect;
	};

	//--------- variables ----------------------------------------------------

	// 現在のアニメーション
	AnimationKey currentAnimationKey_;
	AnimationKey preAnimationKey_;
	AnimationKey editAnimationKey_;

	// 斬撃
	std::unique_ptr<GameEffect> slashEffect_;
	std::unique_ptr<GameEffect> rotateSlashEffect_;
	// 回転する剣の周り
	// 1つに出来たらしたい
	Sword leftSword_;
	Sword rightSword_;
	// 地割れ
	std::unique_ptr<GameEffect> groundEffect_;

	// 1段目の攻撃
	Slash firstSlashParam_;
	// 2段目の攻撃
	static const uint32_t secondSlashCount_ = 2;
	uint32_t secondAttackEventIndex_; // 現在のキーイベント
	std::array<Slash, secondSlashCount_> secondSlashParams_;
	// 3段目の攻撃、2つの剣で共通
	Slash thirdSlashParam_;
	Vector3 thirdParticleTranslation_;
	// 4段目の攻撃
	Slash fourthSlashParam_;
	Vector3 fourthGroundTranslation_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// update
	void UpdateAnimationKey(Player& player);
	void UpdateEmit(Player& player);
	void UpdateAlways();
};