#include "BossEnemyAttackCollision.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Data/Transform.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/Algorithm.h>

// imgui
#include <imgui.h>

//============================================================================
//	BossEnemyAttackCollision classMethods
//============================================================================

void BossEnemyAttackCollision::Init() {

	// 形状初期化
	weaponBody_ = bodies_.emplace_back(Collider::AddCollider(CollisionShape::OBB().Default()));
	bodyOffsets_.emplace_back(CollisionShape::OBB().Default());

	// タイプ設定
	// 最初は無効状態
	weaponBody_->SetType(ColliderType::Type_None);
	weaponBody_->SetTargetType(ColliderType::Type_Player);

	AttackParameter attackParameter{};
	attackParameter.windows.emplace_back();
	table_.emplace(BossEnemyState::LightAttack, attackParameter);
	table_.emplace(BossEnemyState::StrongAttack, attackParameter);
}

void BossEnemyAttackCollision::Update(const Transform3D& transform) {

	auto it = table_.find(currentState_);
	if (it == table_.end()) {
		return;
	}
	AttackParameter& parameter = it->second;

	// 攻撃中かどうか
	bool isAttack = std::any_of(parameter.windows.begin(),
		parameter.windows.end(),
		[t = currentTimer_](const TimeWindow& window) {
			return window.on <= t && t < window.off; });

	// 遷移可能な状態の時のみ武器状態にする
	if (isAttack) {

		weaponBody_->SetType(ColliderType::Type_BossWeapon);

		// 状態別で形状の値を設定
		auto& offset = std::get<CollisionShape::OBB>(bodyOffsets_.front());
		const Vector3 offsetWorld =
			transform.GetRight() * parameter.centerOffset.x +
			transform.GetUp() * parameter.centerOffset.y +
			transform.GetForward() * parameter.centerOffset.z;
		offset.center = offsetWorld;
		offset.size = parameter.size;
	} else {

		weaponBody_->SetType(ColliderType::Type_None);

		// 当たらないようにする
		auto& offset = std::get<CollisionShape::OBB>(bodyOffsets_.front());
		offset.center = Vector3(0.0f, -128.0f, 0.0f);
		offset.size = Vector3::AnyInit(0.0f);
	}


	// 時間を進める
	currentTimer_ += GameTimer::GetDeltaTime();

	// 衝突情報更新
	Collider::UpdateAllBodies(transform);
}

void BossEnemyAttackCollision::SetEnterState(BossEnemyState state) {

	currentState_ = state;
	currentTimer_ = 0.0f;
	// 無効状態を設定
	weaponBody_->SetType(ColliderType::Type_None);
}

void BossEnemyAttackCollision::OnCollisionEnter(const CollisionBody* collisionBody) {

	if (collisionBody->GetType() == ColliderType::Type_Player) {
	}
}

void BossEnemyAttackCollision::ImGui() {

	ImGui::Text("currentType: %s", EnumAdapter<ColliderType>::ToString(weaponBody_->GetType()));
	ImGui::Text("currentTimer: %.3f", currentTimer_);

	EnumAdapter<BossEnemyState>::Combo("State", &editingState_);
	AttackParameter& parameter = table_[editingState_];

	ImGui::Separator();

	bool edit = false;
	edit |= ImGui::DragFloat3("centerOffset", &parameter.centerOffset.x, 0.01f);
	edit |= ImGui::DragFloat3("size", &parameter.size.x, 0.01f);

	EditWindowParameter("hitWindow", parameter.windows);

	if (edit) {

		auto& offset = std::get<CollisionShape::OBB>(bodyOffsets_.front());
		offset.center = parameter.centerOffset;
		offset.size = parameter.size;
	}
}

void BossEnemyAttackCollision::ApplyJson(const Json& data) {

	for (const auto& [key, value] : data.items()) {

		BossEnemyState state = GetBossEnemyStateFromName(key);
		AttackParameter parameter;

		parameter.centerOffset = JsonAdapter::ToObject<Vector3>(value["centerOffset"]);
		parameter.size = JsonAdapter::ToObject<Vector3>(value["size"]);
		if (value.contains("hitWindows")) {
			for (auto& w : value["hitWindows"]) {

				TimeWindow time;
				time.on = w.value("onTime", 0.0f);
				time.off = w.value("offTime", 0.0f);
				parameter.windows.emplace_back(time);
			}
		}

		table_[state] = parameter;
	}
}

void BossEnemyAttackCollision::SaveJson(Json& data) {

	for (auto& [state, parameter] : table_) {

		Json& value = data[EnumAdapter<BossEnemyState>::ToString(state)];
		value["centerOffset"] = JsonAdapter::FromObject(parameter.centerOffset);
		value["size"] = JsonAdapter::FromObject(parameter.size);

		{
			Json windowData = Json::array();
			for (auto& w : parameter.windows) {

				Json j;
				j["onTime"] = w.on;
				j["offTime"] = w.off;
				windowData.push_back(j);
			}
			value["hitWindows"] = windowData;
		}
	}
}

BossEnemyState BossEnemyAttackCollision::GetBossEnemyStateFromName(const std::string& name) {

	for (int i = 0; i < static_cast<int>(BossEnemyState::RushAttack); ++i) {
		if (name == EnumAdapter<BossEnemyState>::GetEnumName(i)) {

			return static_cast<BossEnemyState>(i);
		}
	}
	return BossEnemyState::Idle;
}

void BossEnemyAttackCollision::EditWindowParameter(
	const std::string& label, std::vector<TimeWindow>& windows) {

	ImGui::PushID(label.c_str());

	ImGui::SeparatorText(label.c_str());

	for (size_t i = 0; i < windows.size(); ++i) {

		ImGui::PushID(static_cast<int>(i));
		ImGui::DragFloat("onTime", &windows[i].on, 0.01f, 0.0f);
		ImGui::DragFloat("offTime", &windows[i].off, 0.01f, windows[i].on);
		ImGui::PopID();
	}
	if (ImGui::Button(("AddOnTime" + label).c_str())) {

		windows.emplace_back();
	}
	if (!windows.empty() && ImGui::Button(("RemoveOnTime" + label).c_str())) {

		windows.pop_back();
	}

	ImGui::PopID();
}