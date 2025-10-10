#include "BossEnemyAttackSign.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	BossEnemyAttackSign classMethods
//============================================================================

void BossEnemyAttackSign::Init() {

	// 表示スプライトの初期化
	for (auto& sign : signs_) {

		sign = std::make_unique<GameObject2D>();
		sign->Init("redCircle", "bossEnemyAttackSign", "BossEnemyHUD");

		// スプライト設定
		sign->SetSpriteLayer(SpriteLayer::PostModel);
		sign->SetBlendMode(BlendMode::kBlendModeAdd);
		sign->SetSize(Vector2::AnyInit(0.0f));
		// 色
		sign->SetColor(Color::Convert(0xF06C0AFF));
		sign->SetEmissionColor(Vector3(sign->GetColor().r,
			sign->GetColor().g, sign->GetColor().b));
		sign->SetEmissiveIntensity(1.0f);
	}

	// 初期状態
	currentState_ = State::None;

	// json適応
	ApplyJson();
}

void BossEnemyAttackSign::Emit(const Vector2& emitPos) {

	// 発生設定
	emitPos_ = emitPos;
	currentState_ = State::Update;

	// 発生位置、アンカーを設定
	for (uint32_t i = 0; i < static_cast<uint32_t>(SignDirection::Count); ++i) {

		auto& sign = signs_[i];
		const auto& info = GetDirectionInfo(static_cast<SignDirection>(i));
		sign->SetTranslation(emitPos_);
		sign->SetAnchor(info.anchor);
		sign->SetSize(Vector2::AnyInit(0.0f));
	}
}

void BossEnemyAttackSign::Update() {

	switch (currentState_) {
	case BossEnemyAttackSign::State::None: {

		// タイマーを常にリセット
		animationTimer_.Reset();
		break;
	}
	case BossEnemyAttackSign::State::Update: {

		// アニメーションの更新処理
		UpdateAnimation();
		break;
	}
	}
}

void BossEnemyAttackSign::UpdateAnimation() {

	// 時間を更新
	animationTimer_.Update();

	// 値を補間
	const float t = std::clamp(animationTimer_.easedT_, 0.0f, 1.0f);
	const Vector2 baseSize = Vector2::Lerp(size_.start, size_.target, t);
	const float move = std::lerp(move_.start, move_.target, t);

	for (uint32_t i = 0; i < static_cast<uint32_t>(SignDirection::Count); ++i) {

		auto& sign = signs_[i];
		const auto& info = GetDirectionInfo(static_cast<SignDirection>(i));

		// サイズ、横長か縦長で処理を分ける
		const Vector2 size = info.horizontal ? Vector2(baseSize.y, baseSize.x)
			: Vector2(baseSize.x, baseSize.y);

		// 進行方向へ移動
		const Vector2 pos = emitPos_ + info.direction * move;

		sign->SetAnchor(info.anchor);
		sign->SetTranslation(pos);
		sign->SetSize(size);
	}

	// 時間が経過しきれば処理終了
	if (animationTimer_.IsReached()) {

		// 補間終了時点で表示を消す
		for (const auto& sign : signs_) {

			sign->SetSize(Vector2::AnyInit(0.0f));
		}
		currentState_ = State::None;
	}
}

BossEnemyAttackSign::DirectionInfo BossEnemyAttackSign::GetDirectionInfo(SignDirection direction) const {

	switch (direction) {
	case SignDirection::Up:     return { Vector2(0.0f, -1.0f), Vector2::AnyInit(0.5f),  false };
	case SignDirection::Bottom: return { Vector2(0.0f,  1.0f), Vector2::AnyInit(0.5f),  false };
	case SignDirection::Right:  return { Vector2(1.0f, 0.0f), Vector2::AnyInit(0.5f),  true };
	case SignDirection::Left:   return { Vector2(-1.0f, 0.0f), Vector2::AnyInit(0.5f),  true };
	default:                    return { Vector2::AnyInit(0.0f), Vector2::AnyInit(0.5f),  false };
	}
}

void BossEnemyAttackSign::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	if (ImGui::Button("Emit")) {

		Emit(Vector2(Config::kWindowWidthf / 2.0f, Config::kWindowHeightf / 2.0f));
	}

	ImGui::Text("currnetState: %s", EnumAdapter<State>::ToString(currentState_));
	ImGui::Text("emitPos: %.2f,%.2f", emitPos_.x, emitPos_.y);

	ImGui::DragFloat2("sizeStart", &size_.start.x, 0.1f);
	ImGui::DragFloat2("sizeTarget", &size_.target.x, 0.1f);

	ImGui::DragFloat("moveStart", &move_.start, 0.1f);
	ImGui::DragFloat("moveTarget", &move_.target, 0.1f);

	animationTimer_.ImGui("AnimationTimer");
}

void BossEnemyAttackSign::ApplyJson() {

	move_.start = 0.0f;
	move_.target = 0.0f;

	Json data;
	if (!JsonAdapter::LoadCheck("Enemy/Boss/bossEnemyAttackSign.json", data)) {
		return;
	}

	animationTimer_.FromJson(data["AnimationTimer"]);
	size_.start = Vector2::FromJson(data["size_start"]);
	size_.target = Vector2::FromJson(data["size_target"]);

	move_.start = data.value("move_start", 0.0f);
	move_.target = data.value("move_target", 0.0f);
}

void BossEnemyAttackSign::SaveJson() {

	Json data;

	animationTimer_.ToJson(data["AnimationTimer"]);
	data["size_start"] = size_.start.ToJson();
	data["size_target"] = size_.target.ToJson();
	data["move_start"] = move_.start;
	data["move_target"] = move_.target;

	JsonAdapter::Save("Enemy/Boss/bossEnemyAttackSign.json", data);
}