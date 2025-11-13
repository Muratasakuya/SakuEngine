#include "GameDigitDisplay.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Scene/Camera/BaseCamera.h>

//============================================================================
//	GameDigitDisplay classMethods
//============================================================================

void GameDigitDisplay::Init(uint32_t maxDigit, const std::string& textureName,
	const std::string& name, const std::string& groupName) {

	digitSprites_.reserve(maxDigit);

	// 左端の数字
	GameObject2D* firstSprite = digitSprites_.emplace_back(std::make_unique<GameObject2D>()).get();
	firstSprite->Init(textureName, name + "_0", groupName);

	// 画像サイズ設定
	const Vector2 texSize = firstSprite->GetTextureSize();
	digitSize_ = Vector2(texSize.x / 10.0f, texSize.y);
	firstSprite->SetTextureSize(digitSize_);
	firstSprite->SetAnchor(Vector2::AnyInit(0.0f));

	// 残りの桁を作成
	for (uint32_t index = 1; index < maxDigit; ++index) {

		GameObject2D* sprite = digitSprites_.emplace_back(std::make_unique<GameObject2D>()).get();
		sprite->Init(textureName, name + "_" + std::to_string(index), groupName);
		sprite->SetTextureSize(digitSize_);
		sprite->SetAnchor(Vector2::AnyInit(0.0f));
	}
}

void GameDigitDisplay::SetSpriteLayer(SpriteLayer layer) {

	for (uint32_t index = 0; index < digitSprites_.size(); ++index) {

		digitSprites_[index]->SetSpriteLayer(layer);
	}
}

void GameDigitDisplay::SetTranslation(const Vector2& translation, const Vector2& offset) {

	for (uint32_t index = 0; index < digitSprites_.size(); ++index) {

		digitSprites_[index]->SetTranslation(Vector2(
			translation.x + offset.x * index, translation.y + offset.y * index));
	}
}

Vector2 GameDigitDisplay::ProjectToScreen(const Vector3& translation, const BaseCamera& camera) {

	Matrix4x4 viewMatrix = camera.GetViewMatrix();
	Matrix4x4 projectionMatrix = camera.GetProjectionMatrix();

	Vector3 viewPos = Vector3::Transform(translation, viewMatrix);
	Vector3 clipPos = Vector3::Transform(viewPos, projectionMatrix);

	float screenX = (clipPos.x * 0.5f + 0.5f) * Config::kWindowWidthf;
	float screenY = (1.0f - (clipPos.y * 0.5f + 0.5f)) * Config::kWindowHeightf;

	return Vector2(screenX, screenY);
}

void GameDigitDisplay::SetSize(const Vector2& size) {

	for (uint32_t index = 0; index < digitSprites_.size(); ++index) {

		digitSprites_[index]->SetSize(size);
	}
}

void GameDigitDisplay::SetDigitSize(uint32_t digitIndex, const Vector2& size) {

	if (digitIndex < digitSprites_.size()) {

		digitSprites_[digitIndex]->SetSize(size);
	}
}

void GameDigitDisplay::SetAlpha(float alpha) {

	for (uint32_t index = 0; index < digitSprites_.size(); ++index) {

		digitSprites_[index]->SetAlpha(alpha);
	}
}

void GameDigitDisplay::SetEmissive(uint32_t digitIndex, float emissive) {

	if (digitIndex < digitSprites_.size()) {

		digitSprites_[digitIndex]->SetEmissiveIntensity(emissive);
	}
}

void GameDigitDisplay::Update(uint32_t maxDigit, int num) {

	// 最大値にclampする
	const int maxValue = static_cast<int>(std::pow(10, maxDigit) - 1);
	num = std::clamp(num, 0, maxValue);

	// maxDigit桁で0埋めした文字列を取得
	std::ostringstream oss;
	oss << std::setw(maxDigit) << std::setfill('0') << num;
	const std::string numString = oss.str();

	// 桁を計算して設定
	for (uint32_t index = 0; index < maxDigit; ++index) {

		// 桁数
		const int digit = numString[index] - '0';
		digitSprites_[index]->SetTextureLeftTop(Vector2(digitSize_.x * digit, 0.0f));
	}
}