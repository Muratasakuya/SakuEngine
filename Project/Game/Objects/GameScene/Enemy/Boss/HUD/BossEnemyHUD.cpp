#include "BossEnemyHUD.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Random/RandomGenerator.h>

//============================================================================
//	BossEnemyHUD classMethods
//============================================================================

void BossEnemyHUD::InitSprite() {

	// HP背景
	hpBackground_ = std::make_unique<GameObject2D>();
	hpBackground_->Init("enemyHPBackground", "hpBackground", "BossEnemyHUD");

	// HP残量
	hpBar_ = std::make_unique<GameHPBar>();
	hpBar_->Init("enemyHPBar", "whiteAlphaGradation_0", "hpBar", "BossEnemyHUD");

	// 撃破靭性値
	destroyBar_ = std::make_unique<GameHPBar>();
	destroyBar_->Init("enemyDestroyBar", "whiteAlphaGradation_0", "destroyBar", "BossEnemyHUD");

	// 撃破靭性値の数字表示
	destroyNumDisplay_ = std::make_unique<GameDigitDisplay>();
	destroyNumDisplay_->Init(2, "toughnessNumber", "destroyNum", "BossEnemyHUD");

	// 名前文字表示
	nameText_ = std::make_unique<GameObject2D>();
	nameText_->Init("bossName", "bossName", "BossEnemyHUD");

	// ダメージ表示
	damageDisplay_ = std::make_unique<GameDisplayDamage>();
	damageDisplay_->Init("enemyDamageNumber", "BossEnemyHUD", 8, 4);
}

void BossEnemyHUD::Init() {

	// sprite初期化
	InitSprite();

	// json適応
	ApplyJson();
}

void BossEnemyHUD::SetDamage(int damage) {

	// ダメージを設定
	damageDisplay_->SetDamage(damage);
}

void BossEnemyHUD::SetDisable() {

	isDisable_ = true;

	// 全てのα値を0.0fにし、表示を消す
	hpBackground_->SetAlpha(0.0f);
	hpBar_->SetAlpha(0.0f);
	destroyBar_->SetAlpha(0.0f);
	destroyNumDisplay_->SetAlpha(0.0f);
	nameText_->SetAlpha(0.0f);
	damageDisplay_->SetAlpha(0.0f);
}

void BossEnemyHUD::SetValid() {

	// 再度表示
	returnVaild_ = true;
}

void BossEnemyHUD::Update(const BossEnemy& bossEnemy) {

	// sprite更新
	UpdateSprite(bossEnemy);

	// alpha値を表示切替で更新
	UpdateAlpha();
}

void BossEnemyHUD::UpdateSprite(const BossEnemy& bossEnemy) {

	// HP残量を更新
	hpBar_->Update(stats_.currentHP, stats_.maxHP, true);
	// 撃破靭性値を更新
	destroyBar_->Update(stats_.currentDestroyToughness, stats_.maxDestroyToughness, true);

	// 撃破靭性値の数字を更新
	destroyNumDisplay_->Update(2, stats_.currentDestroyToughness);

	// ダメージ表記の更新
	damageDisplay_->Update(bossEnemy, *followCamera_);
}

void BossEnemyHUD::UpdateAlpha() {

	if (!isDisable_ || !returnVaild_) {
		return;
	}

	// 時間を進める
	returnAlphaTimer_ += GameTimer::GetDeltaTime();
	float alpha = returnAlphaTimer_ / returnAlphaTime_;
	alpha = EasedValue(returnAlphaEasingType_, alpha);
	alpha = std::clamp(alpha, 0.0f, 1.0f);

	// alpha値を補間して設定
	hpBackground_->SetAlpha(alpha);
	hpBar_->SetAlpha(alpha);
	destroyBar_->SetAlpha(alpha);
	destroyNumDisplay_->SetAlpha(alpha);
	nameText_->SetAlpha(alpha);
	damageDisplay_->SetAlpha(alpha);

	if (returnAlphaTime_ < returnAlphaTimer_) {

		// 念のため1.0fに固定
		hpBackground_->SetAlpha(1.0f);
		hpBar_->SetAlpha(1.0f);
		destroyBar_->SetAlpha(1.0f);
		destroyNumDisplay_->SetAlpha(1.0f);
		nameText_->SetAlpha(1.0f);
		damageDisplay_->SetAlpha(1.0f);

		// 元に戻ったので処理終了
		returnAlphaTimer_ = 0.0f;

		isDisable_ = false;
		returnVaild_ = false;
	}
}

void BossEnemyHUD::ImGui() {

	if (ImGui::Button("SaveJson...hudParameter.json")) {

		SaveJson();
	}

	if (ImGui::BeginTabBar("BossEnemyHUD")) {
		if (ImGui::BeginTabItem("Init##HUD")) {
			if (hpBackgroundParameter_.ImGui("HPBackground")) {

				hpBackground_->SetTranslation(hpBackgroundParameter_.translation);
			}

			if (hpBarParameter_.ImGui("HPBar")) {

				hpBar_->SetTranslation(hpBarParameter_.translation);
			}

			if (destroyBarParameter_.ImGui("DestroyBar")) {

				destroyBar_->SetTranslation(destroyBarParameter_.translation);
			}

			if (destroyNumParameter_.ImGui("DestroyNum") ||
				ImGui::DragFloat2("destroyNumOffset", &destroyNumOffset_.x, 1.0f) ||
				ImGui::DragFloat2("destroyNumSize", &destroyNumSize_.x, 1.0f)) {

				destroyNumDisplay_->SetTranslation(destroyNumParameter_.translation, destroyNumOffset_);
				destroyNumDisplay_->SetSize(destroyNumSize_);
			}

			if (nameTextParameter_.ImGui("NameText")) {

				nameText_->SetTranslation(nameTextParameter_.translation);
			}
			ImGui::EndTabItem();
		}

		if (isDisable_) {
			if (ImGui::Button("Vaild")) {

				SetValid();
			}
		} else {
			if (ImGui::Button("Disable")) {

				SetDisable();
			}
		}
		ImGui::Text("returnAlphaTimer / returnAlphaTime: %f", returnAlphaTimer_ / returnAlphaTime_);
		ImGui::DragFloat("returnAlphaTime", &returnAlphaTime_, 0.01f);
		Easing::SelectEasingType(returnAlphaEasingType_);

		if (ImGui::BeginTabItem("Damage##HUD")) {

			damageDisplay_->ImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void BossEnemyHUD::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Enemy/Boss/hudParameter.json", data)) {
		return;
	}

	hpBackgroundParameter_.ApplyJson(data["hpBackground"]);
	GameCommon::SetInitParameter(*hpBackground_, hpBackgroundParameter_);

	hpBarParameter_.ApplyJson(data["hpBar"]);
	GameCommon::SetInitParameter(*hpBar_, hpBarParameter_);

	destroyBarParameter_.ApplyJson(data["destroyBar"]);
	GameCommon::SetInitParameter(*destroyBar_, destroyBarParameter_);

	destroyNumParameter_.ApplyJson(data["destroyNum"]);
	destroyNumOffset_ = destroyNumOffset_.FromJson(data["destroyNum"]["numOffset"]);
	destroyNumSize_ = destroyNumSize_.FromJson(data["destroyNum"]["numSize"]);
	destroyNumDisplay_->SetTranslation(destroyNumParameter_.translation, destroyNumOffset_);
	destroyNumDisplay_->SetSize(destroyNumSize_);

	nameTextParameter_.ApplyJson(data["nameText"]);
	GameCommon::SetInitParameter(*nameText_, nameTextParameter_);

	returnAlphaTime_ = JsonAdapter::GetValue<float>(data, "returnAlphaTime_");
	returnAlphaEasingType_ = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "returnAlphaEasingType_"));

	damageDisplay_->ApplyJson(data);
}

void BossEnemyHUD::SaveJson() {

	Json data;

	hpBackgroundParameter_.SaveJson(data["hpBackground"]);
	hpBarParameter_.SaveJson(data["hpBar"]);
	destroyBarParameter_.SaveJson(data["destroyBar"]);
	destroyNumParameter_.SaveJson(data["destroyNum"]);
	data["destroyNum"]["numOffset"] = destroyNumOffset_.ToJson();
	data["destroyNum"]["numSize"] = destroyNumSize_.ToJson();
	nameTextParameter_.SaveJson(data["nameText"]);
	data["returnAlphaTime_"] = returnAlphaTime_;
	data["returnAlphaEasingType_"] = static_cast<int>(returnAlphaEasingType_);

	damageDisplay_->SaveJson(data);

	JsonAdapter::Save("Enemy/Boss/hudParameter.json", data);
}