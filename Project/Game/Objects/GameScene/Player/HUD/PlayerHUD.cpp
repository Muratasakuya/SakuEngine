#include "PlayerHUD.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	PlayerHUD classMethods
//============================================================================

void PlayerHUD::InitSprite() {

	// HP背景
	hpBackground_ = std::make_unique<GameObject2D>();
	hpBackground_->Init("playerHPBackground", "hpBackground", "PlayerHUD");
	hpBackground_->SetSpriteLayer(SpriteLayer::PreModel);

	// HP残量
	hpBar_ = std::make_unique<GameHPBar>();
	hpBar_->Init("playerHPBar", "whiteAlphaGradation_1", "hpBar", "PlayerHUD");
	hpBar_->SetSpriteLayer(SpriteLayer::PreModel);

	// スキル値
	skilBar_ = std::make_unique<GameHPBar>();
	skilBar_->Init("playerSkilBar", "whiteAlphaGradation_1", "destroyBar", "PlayerHUD");
	skilBar_->SetSpriteLayer(SpriteLayer::PreModel);

	// 名前文字表示
	nameText_ = std::make_unique<GameObject2D>();
	nameText_->Init("playerName", "playerName", "PlayerHUD");
	nameText_->SetSpriteLayer(SpriteLayer::PreModel);

	// キーボード操作とパッド操作のtextureの名前を格納する
	std::unordered_map<InputType, std::string> dynamicTextures{};

	// 攻撃
	dynamicTextures[InputType::Keyboard] = "leftMouseClick";
	dynamicTextures[InputType::GamePad] = "XButton";
	attack_.Init(0, "attackIcon", dynamicTextures);

	// ダッシュ
	dynamicTextures[InputType::Keyboard] = "rightMouseClick";
	dynamicTextures[InputType::GamePad] = "AButton";
	dash_.Init(1, "dashIcon", dynamicTextures);

	// スキル
	dynamicTextures[InputType::Keyboard] = "EButton";
	dynamicTextures[InputType::GamePad] = "YButton";
	skil_.Init(2, "skilIcon", dynamicTextures);

	// パリィ
	dynamicTextures[InputType::Keyboard] = "spaceButton";
	dynamicTextures[InputType::GamePad] = "LBAndRBButton";
	parry_.Init(3, "parryIcon", dynamicTextures);

	// ダメージ表示
	damageDisplay_ = std::make_unique<GameDisplayDamage>();
	damageDisplay_->Init("playerDamageNumber", "BossEnemyHUD", 4, 3);
	damageDisplay_->SetSpriteLayer(SpriteLayer::PreModel);

	// input状態を取得
	inputType_ = Input::GetInstance()->GetType();
	preInputType_ = inputType_;

	// 最初の表示状態を設定
	attack_.ChangeDynamicSprite(inputType_);
	dash_.ChangeDynamicSprite(inputType_);
	skil_.ChangeDynamicSprite(inputType_);
	parry_.ChangeDynamicSprite(inputType_);
}

void PlayerHUD::Init() {

	// sprite初期化
	InitSprite();

	// json適応
	ApplyJson();
}

void PlayerHUD::SetDamage(int damage) {

	// ダメージを設定
	damageDisplay_->SetDamage(damage);
}

void PlayerHUD::SetDisable() {

	isDisable_ = true;

	// 全てのα値を0.0fにし、表示を消す
	hpBackground_->SetAlpha(0.0f);
	hpBar_->SetAlpha(0.0f);
	skilBar_->SetAlpha(0.0f);
	nameText_->SetAlpha(0.0f);
	attack_.SetAlpha(inputType_, 0.0f);
	dash_.SetAlpha(inputType_, 0.0f);
	skil_.SetAlpha(inputType_, 0.0f);
	parry_.SetAlpha(inputType_, 0.0f);
	damageDisplay_->SetAlpha(0.0f);
}

void PlayerHUD::SetValid() {

	// 再度表示
	returnVaild_ = true;
}

void PlayerHUD::Update(const Player& player) {

	// input状態を取得
	inputType_ = Input::GetInstance()->GetType();

	// sprite更新
	UpdateSprite(player);

	// alpha値を表示切替で更新
	UpdateAlpha();
}

void PlayerHUD::UpdateSprite(const Player& player) {

	// HP残量を更新
	hpBar_->Update(stats_.currentHP, stats_.maxHP, true);
	// スキル値を更新
	skilBar_->Update(stats_.currentSkilPoint, stats_.maxSkilPoint, true);

	// ダメージ表記の更新
	damageDisplay_->Update(player, *followCamera_);

	// 入力状態に応じて表示を切り替える
	ChangeAllOperateSprite();
}

void PlayerHUD::UpdateAlpha() {

	// 無効状態でない(表示中)、元に戻す必要がないときは処理しない
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
	skilBar_->SetAlpha(alpha);
	nameText_->SetAlpha(alpha);
	attack_.SetAlpha(inputType_, alpha);
	dash_.SetAlpha(inputType_, alpha);
	skil_.SetAlpha(inputType_, alpha);
	parry_.SetAlpha(inputType_, alpha);
	damageDisplay_->SetAlpha(alpha);

	if (returnAlphaTime_ < returnAlphaTimer_) {

		// 念のため1.0fに固定
		hpBackground_->SetAlpha(1.0f);
		hpBar_->SetAlpha(1.0f);
		skilBar_->SetAlpha(1.0f);
		nameText_->SetAlpha(1.0f);
		attack_.SetAlpha(inputType_, 1.0f);
		dash_.SetAlpha(inputType_, 1.0f);
		skil_.SetAlpha(inputType_, 1.0f);
		parry_.SetAlpha(inputType_, 1.0f);
		damageDisplay_->SetAlpha(1.0f);

		// 元に戻ったので処理終了
		returnAlphaTimer_ = 0.0f;

		isDisable_ = false;
		returnVaild_ = false;
	}
}

void PlayerHUD::ChangeAllOperateSprite() {

	if (preInputType_ == inputType_) {
		return;
	}

	attack_.ChangeDynamicSprite(inputType_);
	dash_.ChangeDynamicSprite(inputType_);
	skil_.ChangeDynamicSprite(inputType_);
	parry_.ChangeDynamicSprite(inputType_);

	preInputType_ = inputType_;
}

void PlayerHUD::SetAllOperateTranslation() {

	attack_.SetTranslation(leftSpriteTranslation_,
		dynamicSpriteOffsetY_, operateSpriteSpancingX_);

	dash_.SetTranslation(leftSpriteTranslation_,
		dynamicSpriteOffsetY_, operateSpriteSpancingX_);

	skil_.SetTranslation(leftSpriteTranslation_,
		dynamicSpriteOffsetY_, operateSpriteSpancingX_);

	parry_.SetTranslation(leftSpriteTranslation_,
		dynamicSpriteOffsetY_, operateSpriteSpancingX_);
}

void PlayerHUD::SetAllOperateSize() {

	attack_.SetSize(staticSpriteSize_, dynamicSpriteSize_);
	dash_.SetSize(staticSpriteSize_, dynamicSpriteSize_);
	skil_.SetSize(staticSpriteSize_, dynamicSpriteSize_);

	// 入力画像は2倍する
	parry_.SetSize(staticSpriteSize_, Vector2(dynamicSpriteSize_.x * 2.0f, dynamicSpriteSize_.y));
}

void PlayerHUD::ImGui() {

	if (ImGui::Button("SaveJson...hudParameter.json")) {

		SaveJson();
	}

	if (hpBackgroundParameter_.ImGui("HPBackground")) {

		hpBackground_->SetTranslation(hpBackgroundParameter_.translation);
	}

	if (hpBarParameter_.ImGui("HPBar")) {

		hpBar_->SetTranslation(hpBarParameter_.translation);
	}

	if (skilBarParameter_.ImGui("SkilBar")) {

		skilBar_->SetTranslation(skilBarParameter_.translation);
	}


	if (nameTextParameter_.ImGui("NameText")) {

		nameText_->SetTranslation(nameTextParameter_.translation);
	}

	damageDisplay_->ImGui();

	ImGui::Separator();

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

	bool edit = false;

	edit |= ImGui::DragFloat2("leftSpriteTranslation", &leftSpriteTranslation_.x, 1.0f);
	edit |= ImGui::DragFloat("dynamicSpriteOffsetY", &dynamicSpriteOffsetY_, 1.0f);
	edit |= ImGui::DragFloat("operateSpriteSpancingX", &operateSpriteSpancingX_, 1.0f);
	ImGui::DragFloat("returnAlphaTime", &returnAlphaTime_, 0.01f);
	Easing::SelectEasingType(returnAlphaEasingType_);

	if (edit) {

		SetAllOperateTranslation();
	}

	edit |= ImGui::DragFloat2("staticSpriteSize", &staticSpriteSize_.x, 0.1f);
	edit |= ImGui::DragFloat2("dynamicSpriteSize", &dynamicSpriteSize_.x, 0.1f);

	if (edit) {

		SetAllOperateSize();
	}
}

void PlayerHUD::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Player/hudParameter.json", data)) {
		return;
	}

	hpBackgroundParameter_.ApplyJson(data["hpBackground"]);
	GameCommon::SetInitParameter(*hpBackground_, hpBackgroundParameter_);

	hpBarParameter_.ApplyJson(data["hpBar"]);
	GameCommon::SetInitParameter(*hpBar_, hpBarParameter_);

	skilBarParameter_.ApplyJson(data["skilBar"]);
	GameCommon::SetInitParameter(*skilBar_, skilBarParameter_);

	nameTextParameter_.ApplyJson(data["nameText"]);
	GameCommon::SetInitParameter(*nameText_, nameTextParameter_);

	damageDisplay_->ApplyJson(data);

	leftSpriteTranslation_ = leftSpriteTranslation_.FromJson(data["leftSpriteTranslation"]);
	staticSpriteSize_ = leftSpriteTranslation_.FromJson(data["staticSpriteSize"]);
	dynamicSpriteSize_ = leftSpriteTranslation_.FromJson(data["dynamicSpriteSize"]);
	dynamicSpriteOffsetY_ = JsonAdapter::GetValue<float>(data, "dynamicSpriteOffsetY");
	operateSpriteSpancingX_ = JsonAdapter::GetValue<float>(data, "operateSpriteSpancingX");
	returnAlphaTime_ = JsonAdapter::GetValue<float>(data, "returnAlphaTime_");
	returnAlphaEasingType_ = static_cast<EasingType>(
		JsonAdapter::GetValue<int>(data, "returnAlphaEasingType_"));

	SetAllOperateTranslation();
	SetAllOperateSize();
}

void PlayerHUD::SaveJson() {

	Json data;

	hpBackgroundParameter_.SaveJson(data["hpBackground"]);
	hpBarParameter_.SaveJson(data["hpBar"]);
	skilBarParameter_.SaveJson(data["skilBar"]);
	nameTextParameter_.SaveJson(data["nameText"]);

	damageDisplay_->SaveJson(data);

	data["leftSpriteTranslation"] = leftSpriteTranslation_.ToJson();
	data["staticSpriteSize"] = staticSpriteSize_.ToJson();
	data["dynamicSpriteSize"] = dynamicSpriteSize_.ToJson();
	data["dynamicSpriteOffsetY"] = dynamicSpriteOffsetY_;
	data["operateSpriteSpancingX"] = operateSpriteSpancingX_;
	data["returnAlphaTime_"] = returnAlphaTime_;
	data["returnAlphaEasingType_"] = static_cast<int>(returnAlphaEasingType_);

	JsonAdapter::Save("Player/hudParameter.json", data);
}

void PlayerHUD::InputStateSprite::Init(uint32_t spriteIndex, const std::string& staticSpriteTextureName,
	const std::unordered_map<InputType, std::string>& dynamicSpritesTextureName) {

	index = spriteIndex;

	// 変化しないspriteの初期化
	staticSprite = std::make_unique<GameObject2D>();
	staticSprite->Init(staticSpriteTextureName, staticSpriteTextureName, groupName);
	staticSprite->SetSpriteLayer(SpriteLayer::PreModel);

	// 変化するspriteをタイプごとに初期化
	for (auto& [type, texture] : dynamicSpritesTextureName) {

		dynamicSprites[type] = std::make_unique<GameObject2D>();
		dynamicSprites[type]->Init(texture, texture, groupName);
		dynamicSprites[type]->SetSpriteLayer(SpriteLayer::PreModel);
	}
}

void PlayerHUD::InputStateSprite::ChangeDynamicSprite(InputType type) {

	// 表示の切り替え
	for (auto& [key, sprite] : dynamicSprites) {
		if (key == type) {

			sprite->SetAlpha(1.0f);
		} else {

			sprite->SetAlpha(0.0f);
		}
	}
}

void PlayerHUD::InputStateSprite::SetTranslation(const Vector2& leftSpriteTranslation,
	float dynamicSpriteOffsetY, float operateSpriteSpancingX) {

	// X座標
	float translationX = leftSpriteTranslation.x + index * operateSpriteSpancingX;

	// 座標を設定
	staticSprite->SetTranslation(Vector2(translationX, leftSpriteTranslation.y));

	dynamicSprites[InputType::Keyboard]->SetTranslation(
		Vector2(translationX, leftSpriteTranslation.y + dynamicSpriteOffsetY));
	dynamicSprites[InputType::GamePad]->SetTranslation(
		Vector2(translationX, leftSpriteTranslation.y + dynamicSpriteOffsetY));
}

void PlayerHUD::InputStateSprite::SetSize(const Vector2& staticSpriteSize,
	const Vector2& dynamicSpriteSize_) {

	// サイズ設定
	staticSprite->SetSize(staticSpriteSize);
	dynamicSprites[InputType::Keyboard]->SetSize(dynamicSpriteSize_);
	dynamicSprites[InputType::GamePad]->SetSize(dynamicSpriteSize_);
}

void PlayerHUD::InputStateSprite::SetAlpha(InputType type, float alpha) {

	// サイズ設定
	staticSprite->SetAlpha(alpha);
	dynamicSprites[type]->SetAlpha(alpha);
}