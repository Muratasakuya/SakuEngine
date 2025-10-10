#include "GameTimerDisplay.h"

//============================================================================
//	GameTimerDisplay classMethods
//============================================================================

void GameTimerDisplay::Init(const std::string& pattern, const std::string& digits,
	const std::string& symbols, const std::string& name, const std::string& groupName, float space) {

	// 画像サイズを取得
	GetDigitSize(digits);
	GetSymbolSize(symbols);

	// パターン別で作成
	float cursorX = 0.0f;
	uint32_t index = 0;
	for (const auto& cPattern : pattern) {

		Element element;
		element.sprite = std::make_unique<GameObject2D>();
		std::string tag = name + "_" + std::to_string(++index);

		// 数字の場合の作成処理
		if (cPattern == 'd') {

			// 数字として初期化
			element.isDigit = true;
			element.sprite->Init(digits, tag, groupName);
			element.sprite->SetSize(digitSize_);
			element.sprite->SetTextureSize(digitSize_);
			element.sprite->SetTextureLeftTop(Vector2::AnyInit(0.0f));
			offsets_.emplace_back(Vector2(cursorX, 0.0f));

			// 次の数字にオフセットを取る
			cursorX += digitSize_.x + space;
		}
		// 記号の場合の作成処理
		else {

			// 記号として初期化
			element.isDigit = false;
			element.sprite->Init(symbols, tag, groupName);
			element.sprite->SetSize(symbolSize_);
			element.sprite->SetTextureSize(symbolSize_);

			auto it = charIndex_.find(cPattern);
			if (it != charIndex_.end()) {

				element.sprite->SetTextureLeftTop(Vector2(symbolSize_.x * it->second, 0.0f));
			} else {

				// 何の記号でない場合エラー
				ASSERT(FALSE, cPattern + ": this symbol is Unusable");
			}
			offsets_.emplace_back(Vector2(cursorX, 0.0f));

			// 次の数字にオフセットを取る
			cursorX += symbolSize_.x + space;
		}
		elements_.emplace_back(std::move(element));
	}

	// 値を保存
	baseOffsets_ = offsets_;
}

void GameTimerDisplay::Update(float second) {

	// センチ秒に変換(四捨五入)
	int centiSecond = static_cast<int>(second * 100.0f + 0.5f);

	const size_t digitNum =
		std::count_if(elements_.begin(), elements_.end(),
			[](const Element& element) { return element.isDigit; });

	// 範囲外の値にならないようにする
	const int maxValue = static_cast<int>(std::pow(10, digitNum) - 1);
	centiSecond = std::clamp(centiSecond, 0, maxValue);

	// 0埋め文字列へ変換する
	std::ostringstream os;
	os << std::setw(digitNum) << std::setfill('0') << centiSecond;
	const std::string digitsString = os.str();

	size_t pos = 0;
	for (const Element& element : elements_) {

		// 記号は数字で変化させないので更新しない
		if (!element.isDigit) {
			continue;
		}

		int digit = digitsString[pos++] - '0';
		element.sprite->SetTextureLeftTop(Vector2(digitSize_.x * digit, 0.0f));
	}
}

void GameTimerDisplay::SetTranslation(const Vector2& translation) {

	for (size_t i = 0; i < elements_.size(); ++i) {

		elements_[i].sprite->SetTranslation(translation + offsets_[i]);
	}
}

void GameTimerDisplay::SetOffset(const Vector2& offset) {

	for (size_t i = 0; i < offsets_.size(); ++i) {

		offsets_[i] = baseOffsets_[i] + Vector2(offset.x * i, offset.y);
	}
}

void GameTimerDisplay::SetElementOffset(uint32_t index, const Vector2& offset) {

	if (index < offsets_.size()) {

		offsets_[index] = offset;
	}
}

void GameTimerDisplay::SetElementSize(uint32_t index, const Vector2& size) {

	if (index < elements_.size()) {

		elements_[index].sprite->SetSize(size);
	}
}

void GameTimerDisplay::SetTimerSize(const Vector2& size) {

	for (size_t i = 0; i < elements_.size(); ++i) {
		if (elements_[i].isDigit) {

			elements_[i].sprite->SetSize(size);
		}
	}
}

void GameTimerDisplay::SetSymbolSize(const Vector2& size) {

	for (size_t i = 0; i < elements_.size(); ++i) {
		if (!elements_[i].isDigit) {

			elements_[i].sprite->SetSize(size);
		}
	}
}

void GameTimerDisplay::SetAlpha(float alpha) {

	for (size_t i = 0; i < elements_.size(); ++i) {

		elements_[i].sprite->SetAlpha(alpha);
	}
}

void GameTimerDisplay::GetDigitSize(const std::string& name) {

	// 仮作成して画像サイズを取得して破棄する
	std::unique_ptr<GameObject2D> dummy = std::make_unique<GameObject2D>();
	dummy->Init(name, name + "Dummy", "DummyGroup");
	const Vector2 textureSize = dummy->GetTextureSize();
	digitSize_ = { textureSize.x / 10.0f, textureSize.y };

	// 画像サイズを取得したので破棄
	dummy.reset();
}

void GameTimerDisplay::GetSymbolSize(const std::string& name) {

	// 仮作成して画像サイズを取得して破棄する
	std::unique_ptr<GameObject2D> dummy = std::make_unique<GameObject2D>();
	dummy->Init(name, name + "Dummy", "DummyGroup");
	const Vector2 textureSize = dummy->GetTextureSize();
	symbolSize_ = { textureSize.x / static_cast<float>(charIndex_.size()),textureSize.y };

	// 画像サイズを取得したので破棄
	dummy.reset();
}