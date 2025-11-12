#include "KeyframeObject3D.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/TagSystem.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

//============================================================================
//	KeyframeObject3D classMethods
//============================================================================

void KeyframeObject3D::Init(const std::string& name) {

	// キーオブジェクト名を設定
	keyObjectName_ = name;

	// デフォルト設定
	currentState_ = State::None;
	isConnectEnds_ = false;
	lerpType_ = LerpKeyframe::Type::Linear;
	isEditUpdate_ = false;
	isDrawKeyframe_ = false;
}

void KeyframeObject3D::StartLerp() {

	// 補間開始
	currentState_ = State::Updating;
	timer_ = 0.0f;
}

void KeyframeObject3D::Update() {

	// None状態なら何もしない
	if (currentState_ == State::None) {
		return;
	}

	// 時間を更新
	float total = (std::max)(keys_.back().time, std::numeric_limits<float>::epsilon());
	timer_ += GameTimer::GetScaledDeltaTime();
	timer_ = std::clamp(timer_, 0.0f, total);
	// 進捗率
	float progress = timer_ / total;

	// 現在の補間位置を更新、現在の区間の補間tを取得して補間
	currentPos_ = LerpKeyframe::GetValue<Vector3>(GetPositions(), GetT(progress), lerpType_);

	// 時間経過で終了
	if (1.0f <= progress) {

		// 最終位置を設定
		currentPos_ = keys_.empty() ? Vector3::AnyInit(0.0f) : keys_.back().pos;

		// リセット
		currentState_ = State::None;
		timer_ = 0.0f;
	}
}

std::unique_ptr<GameObject3D> KeyframeObject3D::CreateKeyObject(const Vector3& pos) {

	// 生成
	std::unique_ptr<GameObject3D> object = std::make_unique<GameObject3D>();
	object->Init(keyModelName_, keyObjectName_, keyGroupName_);

	// 座標を設定
	object->SetTranslation(pos);

	// 親がいれば親を設定
	if (!parentName_.empty()) {

		object->SetParent(*parent_);
	}

	// 描画設定、シーンにしか表示しない
	object->SetMeshRenderView(isDrawKeyframe_ ? MeshRenderView::Scene : MeshRenderView::None);
	object->SetBlendMode(BlendMode::kBlendModeAdd);
	object->SetScale(Vector3::AnyInit(2.4f));
	object->SetCastShadow(false);
	object->SetShadowRate(1.0f);

	return object;
}

std::vector<Vector3> KeyframeObject3D::GetPositions() const {

	std::vector<Vector3> positions{};
	positions.reserve(keys_.size());
	for (const auto& key : keys_) {

		positions.emplace_back(key.pos);
	}
	return positions;
}

float KeyframeObject3D::GetT(float currentT) const {

	// 2つ未満のキーなら0.0fを返す
	if (keys_.size() < 2) {
		return 0.0f;
	}

	// ノット列
	// 最後のキーの時間が合計
	float total = keys_.back().time;
	std::vector<float> knot;
	knot.reserve(keys_.size());
	for (const auto& key : keys_) {

		// ノット値を計算して追加
		knot.emplace_back(std::clamp(key.time / total, 0.0f, 1.0f));
	}

	// 属する区間を探す
	size_t i = 0;
	while (i + 1 < knot.size() && !(knot[i] <= currentT && currentT <= knot[i + 1])) {

		++i;
	}
	// iが範囲外ならサイズで調整する
	if (knot.size() <= i + 1) {

		i = knot.size() - 2;
	}

	// 区間内tをキーのイージングで補間
	float localT = (currentT - knot[i]) / (std::max)(std::numeric_limits<float>::epsilon(), knot[i + 1] - knot[i]);
	float easedLocalT = EasedValue(keys_[i].easeType, std::clamp(localT, 0.0f, 1.0f));

	// 全体tを計算して返す
	float resultT = (i + easedLocalT) / static_cast<float>(keys_.size() - 1);
	return resultT;
}

void KeyframeObject3D::ImGui() {

	ImGui::PushItemWidth(200.0f);

	// エディター内で更新を呼びだす
	if (isEditUpdate_) {

		Update();
	}

	ImGui::SeparatorText("Config");

	if (!keys_.empty()) {

		ImGui::Text("timer: %.2f / %.2f", timer_, keys_.back().time);

		float total = (std::max)(keys_.back().time, std::numeric_limits<float>::epsilon());
		float progress = timer_ / total;
		ImGui::Text("progress: %.2f", progress);
	}

	ImGui::Checkbox("isEditUpdate", &isEditUpdate_);
	if (ImGui::Checkbox("isDrawKeyframe", &isDrawKeyframe_)) {

		// キーオブジェクトの描画設定を更新
		for (const auto& keyObject : keyObjects_) {

			keyObject->SetMeshRenderView(isDrawKeyframe_ ?
				MeshRenderView::Scene : MeshRenderView::None);
		}
	}

	// キーオブジェクトの追加
	if (ImGui::Button("Add Keyframe")) {

		// キーオブジェクトを生成
		keyObjects_.emplace_back(std::move(CreateKeyObject(
			keys_.empty() ? Vector3(0.0f, 16.0f, 0.0f) : keys_.back().pos)));

		// キー位置を追加
		// 最後のキー位置をコピー
		Key key{};
		key.pos = keyObjects_.back()->GetTransform().GetWorldPos();
		key.pos.y += 4.0f;
		keys_.emplace_back(key);
	}
	// 開始
	if (ImGui::Button("Start")) {

		StartLerp();
	}

	if (ImGui::CollapsingHeader("Parameter")) {

		ImGui::Checkbox("isConnectEnds", &isConnectEnds_);
		EnumAdapter<LerpKeyframe::Type>::Combo("LerpType", &lerpType_);

		ImGui::SeparatorText("Keys");

		for (uint32_t i = 0; i < keys_.size(); ++i) {

			ImGui::PushID(i);

			ImGui::DragFloat("Time", &keys_[i].time, 0.01f, 0.0f);
			Easing::SelectEasingType(keys_[i].easeType);

			if (ImGui::Button("Remove Keyframe")) {

				// キーオブジェクトを削除
				keyObjects_.erase(keyObjects_.begin() + i);
				keys_.erase(keys_.begin() + i);
				ImGui::PopID();
				break;
			}
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Set Parent")) {

		uint32_t currentId = 0;
		ObjectManager* objectManager = ObjectManager::GetInstance();
		// 現在選択されているオブジェクトIDを設定
		for (const auto& [id, tagPtr] : objectManager->GetSystem<TagSystem>()->Tags()) {
			if (objectManager->GetData<Transform3D>(id) == parent_) {

				currentId = id;
				break;
			}
		}

		std::string selectName = parentName_;
		if (ImGuiHelper::SelectTagTarget("Select Follow Target", &currentId, &selectName)) {

			// 親Transformと名前を更新
			parent_ = objectManager->GetData<Transform3D>(currentId);
			// 変更があったことにする
			parent_->SetIsDirty(true);
			parentName_ = selectName;

			// キーオブジェクトの親を更新
			for (const auto& keyObject : keyObjects_) {

				keyObject->SetParent(*parent_);
			}
		}
	}

	// 座標に変更があれば更新
	for (size_t i = 0; i < keyObjects_.size(); ++i) {

		// 座標を比較して変更があれば更新
		Vector3 worldPos = keyObjects_[i]->GetTransform().GetWorldPos();
		if (worldPos != keys_[i].pos) {

			// 座標を更新
			keys_[i].pos = worldPos;
		}
	}
	// 線描画
	LerpKeyframe::DrawKeyframeLine(GetPositions(), lerpType_, isConnectEnds_);
	// 現在の時間の点の位置
	LineRenderer::GetInstance()->DrawSphere(6, 4.0f, currentPos_, Color::Yellow());

	ImGui::PopItemWidth();
}

void KeyframeObject3D::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	// キー位置を取得
	for (const auto& keyJson : data["Keys"]) {

		Key key{};

		key.pos = Vector3::FromJson(keyJson.value("pos", Json()));
		key.time = keyJson.value("time", 0.0f);
		key.easeType = EnumAdapter<EasingType>::FromString(keyJson.value("ease", "Linear")).value();

		keys_.emplace_back(key);
	}

	parentName_ = data.value("parentName_", "");
	lerpType_ = EnumAdapter<LerpKeyframe::Type>::FromString(data.value("lerpType_", "Linear")).value();
	isConnectEnds_ = data.value("isConnectEnds_", false);

	// 親Transformを設定
	if (!parentName_.empty()) {

		ObjectManager* objectManager = ObjectManager::GetInstance();
		TagSystem* tagSystem = objectManager->GetSystem<TagSystem>();
		// システムの更新をさせる
		tagSystem->Update(*objectManager->GetObjectPoolManager());

		for (const auto& [id, tagPtr] : tagSystem->Tags()) {

			// 添え字の数字は考慮しない
			if (tagPtr && tagPtr->identifier == parentName_) {

				parent_ = objectManager->GetData<Transform3D>(id);
				break;
			}
		}
	}

	// キーオブジェクトを生成
	for (const auto& key : keys_) {

		keyObjects_.emplace_back(std::move(CreateKeyObject(key.pos)));
	}
}

void KeyframeObject3D::ToJson(Json& data) {

	for (const auto& key : keys_) {

		Json keyJson;

		keyJson["pos"] = key.pos.ToJson();
		keyJson["time"] = key.time;
		keyJson["ease"] = EnumAdapter<EasingType>::ToString(key.easeType);

		data["Keys"].emplace_back(keyJson);
	}

	data["parentName_"] = parentName_;
	data["lerpType_"] = EnumAdapter<LerpKeyframe::Type>::ToString(lerpType_);
	data["isConnectEnds_"] = isConnectEnds_;
}