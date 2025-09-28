#include "FieldBoundary.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	FieldBoundary classMethods
//============================================================================

void FieldBoundary::Init() {

	// json適応
	ApplyJson();

	// 1度だけ更新
	UpdateAllCollisionBody();
}

void FieldBoundary::SetPushBackTarget(Player* player, BossEnemy* bossEnemy) {

	player_ = nullptr;
	player_ = player;

	bossEnemy_ = nullptr;
	bossEnemy_ = bossEnemy;

	for (const auto& collision : collisions_) {

		collision->SetPushBackTarget(player_, bossEnemy_);
	}
}

void FieldBoundary::UpdateAllCollisionBody() {

	// 全ての衝突を更新
	for (const auto& collision : collisions_) {

		collision->Update();
	}
}

void FieldBoundary::ControlTargetMove() {

	// 座標を制限する
	float clampSize = moveClampLength_ / 2.0f;

	// プレイヤー
	Vector3 translation = player_->GetTranslation();
	translation.x = std::clamp(translation.x, -clampSize, clampSize);
	translation.z = std::clamp(translation.z, -clampSize, clampSize);
	player_->SetTranslation(translation);

	// 敵
	translation = bossEnemy_->GetTranslation();
	translation.x = std::clamp(translation.x, -clampSize, clampSize);
	translation.z = std::clamp(translation.z, -clampSize, clampSize);
	bossEnemy_->SetTranslation(translation);

	// 線
	LineRenderer::GetInstance()->DrawSquare(moveClampLength_,
		Vector3(0.0f, 2.0f, 0.0f), Color::Yellow());
}

void FieldBoundary::ImGui() {

	if (ImGui::Button("Save")) {
		SaveJson();
	}

	ImGui::SameLine();

	if (ImGui::Button("Add FieldWallCollision")) {

		auto collision = std::make_unique<FieldWallCollision>();
		collision->Init();
		collision->SetPushBackTarget(player_, bossEnemy_);
		collisions_.push_back(std::move(collision));
	}

	for (uint32_t i = 0; i < collisions_.size(); ++i) {

		ImGui::PushID(static_cast<int>(i));
		if (ImGui::Button("Remove")) {

			collisions_.erase(collisions_.begin() + i);
			ImGui::PopID();
			--i;
			continue;
		}
		collisions_[i]->ImGui(i);
		collisions_[i]->Update();
		ImGui::PopID();
	}

	// 値操作中にのみ更新
	UpdateAllCollisionBody();
}

void FieldBoundary::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Level/fieldCollisionCollection.json", data)) {
		return;
	}

	if (data.is_array()) {
		for (const auto& it : data) {

			auto collision = std::make_unique<FieldWallCollision>();
			collision->Init();
			collision->FromJson(it);
			collision->SetPushBackTarget(player_, bossEnemy_);
			collision->Update();
			collisions_.push_back(std::move(collision));
		}
	}

	// config
	{
		data.clear();
		if (JsonAdapter::LoadCheck("GameConfig/gameConfig.json", data)) {

			moveClampLength_ = JsonAdapter::GetValue<float>(data["playableArea"], "length");
		}
	}
}

void FieldBoundary::SaveJson() {

	// collision
	{
		Json data = Json::array();
		for (const auto& collision : collisions_) {

			Json one;
			collision->ToJson(one);
			data.push_back(std::move(one));
		}

		JsonAdapter::Save("Level/FieldWallCollisionCollection.json", data);
	}
}