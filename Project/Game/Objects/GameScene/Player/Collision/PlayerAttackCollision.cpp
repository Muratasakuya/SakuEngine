#include "PlayerAttackCollision.h"

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
//	PlayerAttackCollision classMethods
//============================================================================

void PlayerAttackCollision::Init() {

	// 形状初期化
	weaponBody_ = bodies_.emplace_back(Collider::AddCollider(CollisionShape::OBB().Default()));
	bodyOffsets_.emplace_back(CollisionShape::OBB().Default());

	// タイプ設定
	// 最初は無効状態
	weaponBody_->SetType(ColliderType::Type_None);
	weaponBody_->SetTargetType(ColliderType::Type_BossEnemy);
}

void PlayerAttackCollision::Update(const Transform3D& transform) {

	if (!currentParameter_) {
		return;
	}

	transform_ = &transform;

	// 攻撃中かどうか
	bool isAttack = std::any_of(currentParameter_->windows.begin(),
		currentParameter_->windows.end(),
		[t = currentTimer_](const TimeWindow& window) {
			return window.on <= t && t < window.off; });
	reHitTimer_ = (std::max)(0.0f, reHitTimer_ - GameTimer::GetDeltaTime());

	// 遷移可能な状態の時のみ武器状態にする
	if (isAttack && reHitTimer_ <= 0.0f) {

		weaponBody_->SetType(ColliderType::Type_PlayerWeapon);
		if (currentParameter_) {

			// 状態別で形状の値を設定
			auto& offset = std::get<CollisionShape::OBB>(bodyOffsets_.front());

			const Vector3 offsetWorld =
				transform.GetRight() * currentParameter_->centerOffset.x +
				transform.GetUp() * currentParameter_->centerOffset.y +
				transform.GetForward() * currentParameter_->centerOffset.z;
			offset.center = offsetWorld;
			offset.size = currentParameter_->size;
		}
	} else {

		weaponBody_->SetType(ColliderType::Type_None);

		// 当たらないようにする
		auto& offset = std::get<CollisionShape::OBB>(bodyOffsets_.front());
		offset.center = Vector3(0.0f, -128.0f, 0.0f);
		offset.size = Vector3::AnyInit(0.0f);
	}

	// 時間を進める
	currentTimer_ += GameTimer::GetScaledDeltaTime();

	// 衝突情報更新
	Collider::UpdateAllBodies(transform);
}

void PlayerAttackCollision::SetEnterState(PlayerState state) {

	currentTimer_ = 0.0f;
	if (Algorithm::Find(table_, state)) {

		currentParameter_ = &table_[state];
		reHitTimer_ = 0.0f;
	}

	// 無効状態を設定
	weaponBody_->SetType(ColliderType::Type_None);
}

void PlayerAttackCollision::OnCollisionEnter(const CollisionBody* collisionBody) {

	if (currentParameter_ == NULL) {
		return;
	}

	if ((collisionBody->GetType() & ColliderType::Type_BossEnemy) != ColliderType::Type_None) {

		// 多段ヒットクールタイム設定
		reHitTimer_ = currentParameter_->hitInterval;

		// ヒットストップを発生させる
		GameTimer::StartHitStop(currentParameter_->waitTime, currentParameter_->timeScale);
	}
}

void PlayerAttackCollision::ImGui() {

	ImGui::Text("currentType: %s", EnumAdapter<ColliderType>::ToString(weaponBody_->GetType()));

	EnumAdapter<PlayerState>::Combo("EditDamage", &editingState_);
	AttackParameter& parameter = table_[editingState_];

	ImGui::Separator();

	ImGui::Text("reHitTimer: %f / %f", reHitTimer_, parameter.hitInterval);

	bool edit = false;
	edit |= ImGui::DragFloat3("centerOffset", &parameter.centerOffset.x, 0.01f);
	edit |= ImGui::DragFloat3("size", &parameter.size.x, 0.01f);
	edit |= ImGui::DragFloat("hitInterval", &parameter.hitInterval, 0.01f);

	ImGui::SeparatorText("Hit Stop");

	ImGui::DragFloat("timeScale", &parameter.timeScale, 0.01f);
	ImGui::DragFloat("waitTime", &parameter.waitTime, 0.01f);
	ImGui::DragFloat("lerpSpeed", &parameter.lerpSpeed, 0.01f);
	Easing::SelectEasingType(parameter.timeScaleEasing, "TimeScale");

	EditWindowParameter("hitWindows", parameter.windows);

	if (edit) {

		auto& offset = std::get<CollisionShape::OBB>(bodyOffsets_.front());
		offset.center = parameter.centerOffset;
		offset.size = parameter.size;
	}
}

void PlayerAttackCollision::ApplyJson(const Json& data) {

	for (const auto& [key, value] : data.items()) {

		PlayerState state = GetPlayerStateFromName(key);
		if (state == PlayerState::None) {
			continue;
		}
		AttackParameter parameter;

		parameter.centerOffset = JsonAdapter::ToObject<Vector3>(value["centerOffset"]);
		parameter.size = JsonAdapter::ToObject<Vector3>(value["size"]);
		parameter.hitInterval = value.value("hitInterval", 0.0f);
		parameter.timeScale = value.value("timeScale", 1.0f);
		parameter.waitTime = value.value("waitTime", 0.08f);
		parameter.lerpSpeed = value.value("lerpSpeed", 1.0f);
		parameter.timeScaleEasing = EnumAdapter<EasingType>::FromString(value.value("timeScaleEasing", "Linear")).value();
		parameter.hitInterval = value.value("hitInterval", 0.0f);

		if (value.contains("hitWindows")) {
			for (auto& window : value["hitWindows"]) {

				TimeWindow timeWindow{};
				timeWindow.on = window.value("onTime", 0.0f);
				timeWindow.off = window.value("offTime", 0.0f);
				parameter.windows.emplace_back(timeWindow);
			}
		} else {

			TimeWindow timeWindow{};
			timeWindow.on = value.value("onTime", 0.0f);
			timeWindow.off = value.value("offTime", 0.0f);
			parameter.windows.emplace_back(timeWindow);
		}
		table_[state] = parameter;
	}
}

void PlayerAttackCollision::SaveJson(Json& data) {

	for (auto& [state, parameter] : table_) {
		if (state == PlayerState::None) {
			continue;
		}

		Json& value = data[EnumAdapter<PlayerState>::ToString(state)];
		value["centerOffset"] = JsonAdapter::FromObject(parameter.centerOffset);
		value["size"] = JsonAdapter::FromObject(parameter.size);
		value["hitInterval"] = parameter.hitInterval;
		value["timeScale"] = parameter.timeScale;
		value["waitTime"] = parameter.waitTime;
		value["lerpSpeed"] = parameter.lerpSpeed;
		value["timeScaleEasing"] = EnumAdapter<EasingType>::ToString(parameter.timeScaleEasing);

		Json windowData = Json::array();
		{
			for (auto& window : parameter.windows) {

				Json data_;
				data_["onTime"] = window.on;
				data_["offTime"] = window.off;
				windowData.push_back(data_);
			}
		}
		value["hitWindows"] = windowData;
	}
}

PlayerState PlayerAttackCollision::GetPlayerStateFromName(const std::string& name) {

	for (int i = 0; i < static_cast<int>(PlayerState::StunAttack) + 1; ++i) {
		if (name == EnumAdapter<PlayerState>::GetEnumName(i)) {

			return static_cast<PlayerState>(i);
		}
	}
	return PlayerState::None;
}

void PlayerAttackCollision::EditWindowParameter(
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