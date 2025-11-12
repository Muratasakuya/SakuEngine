#include "KeyframeObject3D.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/LineRenderer.h>
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Object/System/Systems/TagSystem.h>
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>
#include <Engine/Input/Input.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>

// imgui
#include <imgui_internal.h>

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

	ImGui::SeparatorText("Key Timer");

	if (!keys_.empty()) {

		ImGui::Text("timer: %.2f / %.2f", timer_, keys_.back().time);

		float total = (std::max)(keys_.back().time, std::numeric_limits<float>::epsilon());
		float progress = timer_ / total;
		ImGui::Text("progress: %.2f", progress);

		// キータイムラインの描画
		DrawKeyTimeline();
	}

	ImGui::SeparatorText("Config");

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

		// キー位置を追加
		// 最後のキー位置をコピー
		Key key{};
		key.pos = keyObjects_.back()->GetTransform().GetWorldPos();
		key.pos.y += 4.0f;
		keys_.emplace_back(key);

		// キーオブジェクトを生成
		keyObjects_.emplace_back(std::move(CreateKeyObject(
			keys_.empty() ? Vector3(0.0f, 16.0f, 0.0f) : keys_.back().pos)));
	}
	// 開始
	if (ImGui::Button("Start")) {

		StartLerp();
	}

	if (ImGui::CollapsingHeader("Parameter")) {

		ImGui::Checkbox("isConnectEnds", &isConnectEnds_);
		EnumAdapter<LerpKeyframe::Type>::Combo("LerpType", &lerpType_);
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

	// Deleteキー入力でエディターで操作中のキーを削除する
	const std::optional<uint32_t> editObjectId = ImGuiObjectEditor::GetInstance()->GetSelected3D();
	if (editObjectId.has_value() && Input::GetInstance()->TriggerKey(DIK_DELETE)) {

		// 選択IDをチェックする
		for (uint32_t i = 0; i < keyObjects_.size(); ++i) {
			if (keyObjects_[i]->GetObjectID() == editObjectId.value()) {

				// キーオブジェクトを削除
				keyObjects_.erase(keyObjects_.begin() + i);
				// キー情報を削除
				keys_.erase(keys_.begin() + i);
				break;
			}
		}
	}
}

void KeyframeObject3D::DrawKeyTimeline() {

	// 表示バーのサイズ
	const float barWidth = 520.0f;
	const float barHeight = 12.0f;

	if (keys_.empty()) {
		return;
	}

	const float total = (std::max)(keys_.back().time, std::numeric_limits<float>::epsilon());

	// レイアウト領域を確保
	ImGui::Dummy(ImVec2(barWidth, barHeight * 2.0f));
	ImVec2 p0 = ImGui::GetItemRectMin();
	ImVec2 p1 = ImGui::GetItemRectMax();
	bool hoveredTimeline = ImGui::IsItemHovered();
	ImDrawList* dl = ImGui::GetWindowDrawList();

	// 背景
	dl->AddRectFilled(p0, p1, IM_COL32(70, 70, 70, 255), barHeight * 0.5f);

	// 進捗バー
	float progT = 0.0f;
	if (currentState_ != State::None) {

		progT = std::clamp(timer_ / total, 0.0f, 1.0f);
	}
	ImVec2 pProg = ImVec2(std::lerp(p0.x, p1.x, progT), p1.y);
	dl->AddRectFilled(p0, pProg, IM_COL32(240, 200, 0, 255), barHeight * 0.5f);

	// 丸の描画とドラッグ
	const float yCenter = (p0.y + p1.y) * 0.5f;
	const float radius = barHeight * 0.7f;

	// 状態保持
	static int s_dragIndex = -1;
	static bool s_dragging = false;
	static int s_easeSeg = -1;

	// 先に丸のヒット＆ドラッグ処理
	ImVec2 mouse = ImGui::GetIO().MousePos;
	bool anyHovered = false;

	for (int i = 0; i < (int)keys_.size(); ++i) {

		float t = (total > 0.0f) ? (keys_[i].time / total) : 0.0f;
		t = std::clamp(t, 0.0f, 1.0f);
		float x = std::lerp(p0.x, p1.x, t);
		ImVec2 center(x, yCenter);

		// 通過済み → 緑 / 未来 → 灰
		ImU32 col = (timer_ >= keys_[i].time) ? IM_COL32(50, 220, 70, 255) : IM_COL32(180, 180, 180, 255);

		// ヒット判定
		bool hovered = ImLengthSqr(mouse - center) <= (radius * radius);
		if (hovered) {

			anyHovered = true;
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			dl->AddCircleFilled(center, radius, IM_COL32(255, 255, 255, 255));
		}

		// ドラッグ開始
		if (!s_dragging && hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			s_dragging = true;
			s_dragIndex = i;
		}

		// 描画
		dl->AddCircleFilled(center, radius * 0.8f, col);

		// ドラッグ中の更新
		if (s_dragging && s_dragIndex == i) {
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {

				// マウスx → 時間（絶対秒）に写像
				float u = 0.0f;
				if (p1.x > p0.x) u = (mouse.x - p0.x) / (p1.x - p0.x);
				u = std::clamp(u, 0.0f, 1.0f);
				float newTime = u * (std::max)(total, 1e-6f);

				// 単調性を維持
				const float minGap = 1e-4f;
				float lo = (i == 0) ? 0.0f : (keys_[i - 1].time + minGap);
				float hi = (i + 1 < (int)keys_.size()) ? (keys_[i + 1].time - minGap) : (std::max)(newTime, lo);
				newTime = std::clamp(newTime, lo, hi);

				// 更新
				keys_[i].time = newTime;

				// ラベル
				char buf[64];
				snprintf(buf, sizeof(buf), "%.3f s", keys_[i].time);
				ImVec2 labelPos(center.x - ImGui::CalcTextSize(buf).x * 0.5f, yCenter + radius + 4.0f);
				dl->AddText(labelPos, IM_COL32(255, 255, 255, 255), buf);
			} else {

				// マウスを離したら終了
				s_dragging = false;
				s_dragIndex = -1;
			}
		}
	}

	// 最後の時間はdragFloatで更新
	ImGui::DragFloat("End Time", &keys_.back().time, 0.01f);

	// 丸以外クリックで区間を選択 → イージング選択ポップアップを開く
	if (!s_dragging && hoveredTimeline && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !anyHovered) {

		// クリック位置 → t → どの区間か
		float u = (p1.x > p0.x) ? (mouse.x - p0.x) / (p1.x - p0.x) : 0.0f;
		u = std::clamp(u, 0.0f, 1.0f);

		// キーの正規化時刻配列を作る
		int seg = -1;
		if (keys_.size() >= 2) {
			for (int i = 0; i + 1 < (int)keys_.size(); ++i) {

				float a = std::clamp(keys_[i].time / total, 0.0f, 1.0f);
				float b = std::clamp(keys_[i + 1].time / total, 0.0f, 1.0f);
				if (a <= u && u <= b) {
					seg = i;
					break;
				}
			}
			if (seg < 0) {

				seg = (int)keys_.size() - 2;
			}
		}
		s_easeSeg = seg;
		ImGui::OpenPopup("EasePopup");
	}

	// ポップアップでイージング選択
	if (ImGui::BeginPopup("EasePopup")) {
		if (0 <= s_easeSeg && s_easeSeg + 1 < (int)keys_.size()) {

			ImGui::Text("Segment: %d -> %d", s_easeSeg, s_easeSeg + 1);
			Easing::SelectEasingType(keys_[s_easeSeg].easeType);
			ImGui::Separator();
		}
		if (ImGui::Button("Close")) {

			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
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