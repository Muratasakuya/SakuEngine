#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject2D.h>

// c++
#include <unordered_map>
// front
class Asset;

//============================================================================
//	GameTimerDisplay class
//============================================================================
class GameTimerDisplay {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameTimerDisplay() = default;
	~GameTimerDisplay() = default;

	void Init(const std::string& pattern, const std::string& digits, const std::string& symbols,
		const std::string& name, const std::string& groupName, float space = 0.0f);

	void Update(float second);

	//--------- accessor -----------------------------------------------------

	void SetTranslation(const Vector2& translation);

	void SetOffset(const Vector2& offset);
	void SetElementOffset(uint32_t index, const Vector2& offset);

	void SetElementSize(uint32_t index, const Vector2& size);
	void SetTimerSize(const Vector2& size);
	void SetSymbolSize(const Vector2& size);

	void SetAlpha(float alpha);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 各桁の表示
	struct Element {

		std::unique_ptr<GameObject2D> sprite;
		bool isDigit;
	};

	//--------- variables ----------------------------------------------------

	std::vector<Element> elements_;
	std::vector<Vector2> offsets_;
	std::vector<Vector2> baseOffsets_;

	Vector2 digitSize_;
	Vector2 symbolSize_;

	// コロンとピリオドの数字インデックス
	std::unordered_map<char, int> charIndex_ = { {':',0},{'.',1} };

	//--------- functions ----------------------------------------------------

	void GetDigitSize(const std::string& name);
	void GetSymbolSize(const std::string& name);
};