#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject2D.h>

//============================================================================
//	GameHPBar class
//	体力ゲージを表示、更新する(α値の閾値をPSで棄却して処理、alpha < alphaTexture.a)
//============================================================================
class GameHPBar :
	public GameObject2D {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameHPBar() = default;
	~GameHPBar() = default;

	// 初期化
	void Init(const std::string& textureName, const std::string& alphaTextureName,
		const std::string& name, const std::string& groupName);

	// 更新
	void Update(int current, int max, bool isReverse);
};