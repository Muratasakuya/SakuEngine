#include "GameDisplayDamage.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Random/RandomGenerator.h>

//============================================================================
//	GameDisplayDamage classMethods
//============================================================================

void GameDisplayDamage::Init(const std::string& textureName, const std::string& groupName,
	uint32_t damageDisplayMaxNum, uint32_t damageDigitMaxNum) {

	// 最大数を設定
	damageDisplayMaxNum_ = damageDisplayMaxNum;
	damageDigitMaxNum_ = damageDigitMaxNum;

	// ダメージ表示
	damagePopups_.resize(damageDisplayMaxNum_);
	for (uint32_t index = 0; index < damageDisplayMaxNum_; ++index) {

		damagePopups_[index].digits = std::make_unique<GameDigitDisplay>();
		damagePopups_[index].digits->Init(damageDigitMaxNum_, textureName,
			"damagePopup_" + std::to_string(index), groupName);

		// 表示しない状態
		damagePopups_[index].active = false;
	}
}

void GameDisplayDamage::SetSpriteLayer(SpriteLayer layer) {

	for (uint32_t index = 0; index < damageDisplayMaxNum_; ++index) {

		damagePopups_[index].digits->SetSpriteLayer(layer);
	}
}

void GameDisplayDamage::SetDamage(int damage) {

	receivedDamages_.push_back(std::clamp(damage, 0, 9999));
}

void GameDisplayDamage::SetAlpha(float alpha) {

	for (uint32_t index = 0; index < damageDisplayMaxNum_; ++index) {

		damagePopups_[index].digits->SetAlpha(alpha);
	}
}

void GameDisplayDamage::Update(const GameObject3D& object, const BaseCamera& camera) {

	const float deltaTime = GameTimer::GetDeltaTime();

	// ダメージを受けている間のみ更新
	while (!receivedDamages_.empty()) {

		int damage = receivedDamages_.front();
		receivedDamages_.pop_front();
		auto it = std::find_if(damagePopups_.begin(), damagePopups_.end(),
			[](const DamagePopup& p) { return !p.active; });
		// 上限異常は表示できないようにする
		if (it == damagePopups_.end()) {
			break;
		}

		// 値を設定、初期化
		it->value = damage;
		it->active = true;
		it->timer = 0.0f;
		it->outTimer = 0.0f;

		// スクリーン座標を取得
		Vector2 bossScreen = it->digits->ProjectToScreen(object.GetTranslation(), camera);
		Vector2 randomOffset{
			RandomGenerator::Generate(-damageDisplayPosRandomRange_.x,damageDisplayPosRandomRange_.x),
			RandomGenerator::Generate(-damageDisplayPosRandomRange_.y,damageDisplayPosRandomRange_.y)
		};
		it->basePos = Vector2(bossScreen.x,
			bossScreen.y + bossScreenPosOffsetY_) + randomOffset;

		// 表示する桁の更新
		it->digits->Update(damageDigitMaxNum_, it->value);
	}
	for (auto& popup : damagePopups_) {

		// 表示しない
		if (!popup.active) {
			popup.digits->SetSize(Vector2::AnyInit(0.0f));
			continue;
		}

		totalAppearDuration_ = damageDisplayTime_ + digitDisplayInterval_ * (damageDigitMaxNum_ - 1);
		const float totalStayEnd = totalAppearDuration_ + damageStayTime_;
		float prevTimer = popup.timer;
		popup.timer += deltaTime;
		if (popup.timer < totalAppearDuration_) {

			for (uint32_t i = 0; i < damageDigitMaxNum_; ++i) {

				float localT = (popup.timer - i * digitDisplayInterval_) / damageDisplayTime_;
				localT = std::clamp(localT, 0.0f, 1.0f);
				float eased = EasedValue(damageDisplayEasingType_, localT);

				Vector2 size = Vector2::Lerp(damageDisplayMaxSize_,
					damageDisplaySize_, eased);
				popup.digits->SetDigitSize(i, size);

				float emissive = std::lerp(maxDamageEmissive_, 0.0f, eased);
				popup.digits->SetEmissive(i, emissive);
			}
		} else if (popup.timer < totalStayEnd) {

			// 全て表示した後damageStayTime_分だけ表示する
			for (uint32_t i = 0; i < damageDigitMaxNum_; ++i) {

				popup.digits->SetDigitSize(i, damageDisplaySize_);
			}
		} else {
			if (prevTimer < totalStayEnd) {

				popup.outTimer = 0.0f;
			}

			popup.outTimer += deltaTime;
			float outT = std::clamp(popup.outTimer / damageOutTime_, 0.0f, 1.0f);
			float eased = EasedValue(damageOutEasingType_, outT);
			Vector2 size = Vector2::Lerp(damageDisplaySize_, Vector2::AnyInit(0.0f), eased);
			for (uint32_t i = 0; i < damageDigitMaxNum_; ++i) {

				popup.digits->SetDigitSize(i, size);
				if (popup.outTimer >= damageOutTime_) {

					popup.active = false;
					continue;
				}
			}
		}

		// 座標設定
		popup.digits->SetTranslation(popup.basePos, Vector2(damageDisplaySize_.x + damageNumSpacing_, 0.0f));
	}
}

void GameDisplayDamage::ImGui() {

	ImGui::DragFloat("damageDisplayTime", &damageDisplayTime_, 0.01f);
	ImGui::DragFloat("damageOutTime", &damageOutTime_, 0.01f);
	ImGui::DragFloat("damageStayTime", &damageStayTime_, 0.01f);
	ImGui::DragFloat("damageNumSpacing", &damageNumSpacing_, 0.1f);
	ImGui::DragFloat("digitDisplayInterval", &digitDisplayInterval_, 0.01f);
	ImGui::DragFloat("bossScreenPosOffsetY", &bossScreenPosOffsetY_, 1.0f);
	ImGui::DragFloat("maxDamageEmissive", &maxDamageEmissive_, 1.0f);
	ImGui::DragFloat2("damageDisplayPosRandomRange", &damageDisplayPosRandomRange_.x, 1.0f);
	ImGui::DragFloat2("damageDisplayMaxSize", &damageDisplayMaxSize_.x, 1.0f);
	ImGui::DragFloat2("damageDisplaySize", &damageDisplaySize_.x, 1.0f);

	Easing::SelectEasingType(damageDisplayEasingType_, "damageDisplaySize");
	Easing::SelectEasingType(damageOutEasingType_, "damageOutEasingType");
}

void GameDisplayDamage::ApplyJson(const Json& data) {

	damageDisplayTime_ = JsonAdapter::GetValue<float>(data, "damageDisplayTime_");
	damageOutTime_ = JsonAdapter::GetValue<float>(data, "damageOutTime_");
	damageNumSpacing_ = JsonAdapter::GetValue<float>(data, "damageNumSpacing_");
	digitDisplayInterval_ = JsonAdapter::GetValue<float>(data, "digitDisplayInterval_");
	bossScreenPosOffsetY_ = JsonAdapter::GetValue<float>(data, "bossScreenPosOffsetY_");
	maxDamageEmissive_ = JsonAdapter::GetValue<float>(data, "maxDamageEmissive_");
	damageStayTime_ = JsonAdapter::GetValue<float>(data, "damageStayTime_");
	damageDisplayPosRandomRange_ = JsonAdapter::ToObject<Vector2>(data["damageDisplayPosRandomRange_"]);
	damageDisplayMaxSize_ = JsonAdapter::ToObject<Vector2>(data["damageDisplayMaxSize_"]);
	damageDisplaySize_ = JsonAdapter::ToObject<Vector2>(data["damageDisplaySize_"]);
	damageDisplayEasingType_ = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "damageDisplayEasingType_"));
	damageOutEasingType_ = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "damageOutEasingType_"));

	totalAppearDuration_ = damageDisplayTime_ + digitDisplayInterval_ * (damageDigitMaxNum_ - 1);
}

void GameDisplayDamage::SaveJson(Json& data) {

	data["damageDisplayTime_"] = damageDisplayTime_;
	data["damageOutTime_"] = damageOutTime_;
	data["damageNumSpacing_"] = damageNumSpacing_;
	data["digitDisplayInterval_"] = digitDisplayInterval_;
	data["bossScreenPosOffsetY_"] = bossScreenPosOffsetY_;
	data["maxDamageEmissive_"] = maxDamageEmissive_;
	data["damageStayTime_"] = damageStayTime_;
	data["damageDisplayPosRandomRange_"] = JsonAdapter::FromObject<Vector2>(damageDisplayPosRandomRange_);
	data["damageDisplayMaxSize_"] = JsonAdapter::FromObject<Vector2>(damageDisplayMaxSize_);
	data["damageDisplaySize_"] = JsonAdapter::FromObject<Vector2>(damageDisplaySize_);
	data["damageDisplayEasingType_"] = static_cast<int>(damageDisplayEasingType_);
	data["damageOutEasingType_"] = static_cast<int>(damageOutEasingType_);
}