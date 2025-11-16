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

void KeyframeObject3D::Init(const std::string& name, const std::string& modelName) {

	// キーオブジェクト名を設定
	keyObjectName_ = name;
	keyModelName_ = modelName;

	// デフォルト設定
	addKeyTimeStep_ = 0.8f;
	currentState_ = State::None;
	isConnectEnds_ = false;
	lerpType_ = LerpKeyframe::Type::Linear;

	startDuration_ = 0.0f;
	startEaseType_ = EasingType::Linear;

	isUpdateKeyDuringLerp_ = true;

	isEditUpdate_ = false;
	isDrawKeyframe_ = false;
}

const Transform3D& KeyframeObject3D::GetIndexTransform(uint32_t index) const {

	return keyObjects_[index]->GetTransform();
}

KeyframeObject3D::AnyValue KeyframeObject3D::GetIndexAnyValue(uint32_t index, const std::string& name) const {

	// 名前を探してnameIndex番目の値を返す
	for (size_t nameIndex = 0; nameIndex < anyTracks_.size(); ++nameIndex) {

		// 名前をチェック
		if (anyTracks_[nameIndex].name == name) {

			// index番目のnameIndex番目を返す
			return keys_[index].anyValues[nameIndex];
		}
	}
	return AnyValue{};
}

KeyframeObject3D::AnyValue KeyframeObject3D::GetCurrentAnyValue(const std::string& name) const {

	// 名前を探してindex番目の値を返す
	for (size_t nameIndex = 0; nameIndex < anyTracks_.size(); ++nameIndex) {

		// 名前をチェック
		if (anyTracks_[nameIndex].name == name) {

			// nameIndex番目を返す
			return currentAnyValues_[nameIndex];
		}
	}
	return AnyValue{};
}

std::vector<uint32_t> KeyframeObject3D::GetKeyObjectIDs() const {

	std::vector<uint32_t> ids{};
	for (const auto& keyObject : keyObjects_) {

		ids.emplace_back(keyObject->GetObjectID());
	}
	return ids;
}

uint32_t KeyframeObject3D::GetKeyIndexFromObjectID(uint32_t index) {

	uint32_t keyIndex = 0;
	for (const auto& keyObject : keyObjects_) {
		if (keyObject->GetObjectID() == index) {
			break;
		}
		++keyIndex;
	}
	return keyIndex;
}

void KeyframeObject3D::StartLerp(const std::optional<Transform3D>& transform,
	const std::optional<std::vector<AnyValue>>& anyValues) {

	// 補間中は開始できない
	if (currentState_ == State::Updating) {
		return;
	}

	// ランタイム情報リセット
	runtime_ = Runtime{};

	// 補間開始
	currentState_ = State::Updating;
	timer_ = 0.0f;

	// 最初の補間値が設定されていれば追加
	if (transform.has_value()) {

		runtime_.hasStartKey = true;

		// 最初のトランスフォームを設定
		runtime_.startTransform = transform.value();

		// 任意値の設定
		runtime_.startAnyValues.clear();
		if (anyValues.has_value() && !anyValues->empty()) {

			runtime_.startAnyValues = anyValues.value();
		} else {

			// 何も渡されなかった場合は追加されている数分デフォルト値を入れておく
			runtime_.startAnyValues.resize(anyTracks_.size());
			for (size_t i = 0; i < anyTracks_.size(); ++i) {

				runtime_.startAnyValues[i] = MakeDefaultAnyValue(anyTracks_[i].type);
			}
		}
	}
}

void KeyframeObject3D::Reset() {

	// Noneにする
	currentState_ = State::None;

	// リセット
	timer_ = 0.0f;
	runtime_.hasStartKey = false;
	runtime_.startAnyValues.clear();
}

void KeyframeObject3D::AddKeyValue(AnyMold mold, const std::string& name) {

	// 同じ名前の値は追加できないようにする
	for (const auto& track : anyTracks_) {
		// 名前が同じなかどうか
		if (track.name == name) {
			return;
		}
	}

	// 値を追加
	AnyTrack track{};
	track.type = mold;
	track.name = name;
	anyTracks_.emplace_back(track);
	// 初期値
	AnyValue defaultValue = MakeDefaultAnyValue(mold);

	// 補間する値を追加する
	for (auto& key : keys_) {

		key.anyValues.emplace_back(defaultValue);
	}
	currentAnyValues_.emplace_back(defaultValue);
}

void KeyframeObject3D::SelfUpdate() {

	// 行列の更新は常に行う
	for (auto& key : keys_) {

		key.transform.UpdateMatrix();
	}

	// None状態なら何もしない
	if (currentState_ == State::None) {
		return;
	}

	// 時間を更新
	// keys分の合計時間
	float baseTotal = (std::max)(keys_.back().time, std::numeric_limits<float>::epsilon());
	// 追加されてれば最初の区間の長さを加算
	float startTime = (runtime_.hasStartKey ? startDuration_ : 0.0f);
	float total = baseTotal + startTime;

	// 時間を更新
	timer_ += GameTimer::GetScaledDeltaTime();
	timer_ = std::clamp(timer_, 0.0f, total);

	//============================================================================
	//	最初の区間の補間、start -> key0
	//============================================================================
	if (runtime_.hasStartKey && timer_ < startTime) {

		// 最初の区間のローカルt値
		float local = 0.0f < startTime ? (timer_ / startTime) : 1.0f;
		local = std::clamp(local, 0.0f, 1.0f);
		float easedT = EasedValue(startEaseType_, local);

		// 最初のキーのトランスフォーム
		const Transform3D& key0Transform = keys_.front().transform;

		// トランスフォームを補間
		// S
		currentTransform_.scale = LerpKeyframe::Lerp(runtime_.startTransform.scale, key0Transform.scale, easedT);
		// R
		currentTransform_.rotation = LerpKeyframe::Lerp(runtime_.startTransform.rotation, key0Transform.rotation, easedT);
		// T
		currentTransform_.translation = LerpKeyframe::Lerp(runtime_.startTransform.translation, key0Transform.translation, easedT);

		// 任意値の補間
		UpdateStartAnyValues(easedT);
	}
	//============================================================================
	//	keys間の補間、key.front -> key.back
	//============================================================================
	else {

		// 開始時間を引いた本来の補間時間の進捗率
		float progress = (timer_ - startTime) / baseTotal;
		// 現在のt値を取得
		float currentT = GetT(progress);

		// トランスフォームを補間
		// S
		currentTransform_.scale = LerpKeyframe::GetValue<Vector3>(GetScales(), currentT, lerpType_);
		// R
		currentTransform_.rotation = LerpKeyframe::GetValue<Quaternion>(GetRotations(), currentT, lerpType_);
		// T
		currentTransform_.translation = LerpKeyframe::GetValue<Vector3>(GetPositions(), currentT, lerpType_);

		// 任意値の補間
		UpdateAnyValues(currentT);
	}

	// 時間経過で終了
	if (total <= timer_) {

		// 最後のキーの値をセット
		currentTransform_.scale = keys_.empty() ? Vector3::AnyInit(1.0f) : keys_.back().transform.scale;
		currentTransform_.rotation = keys_.empty() ? Quaternion::Identity() : keys_.back().transform.rotation;
		currentTransform_.translation = keys_.empty() ? Vector3::AnyInit(0.0f) : keys_.back().transform.translation;

		// リセットして終了
		Reset();
	}
}

void KeyframeObject3D::ExternalInputTUpdate(float inputT) {

	// 必ず0.0f~1.0fの間
	float t = std::clamp(inputT, 0.0f, 1.0f);

	// スケール
	currentTransform_.scale = LerpKeyframe::GetValue<Vector3>(GetScales(), t, lerpType_);
	// 回転
	currentTransform_.rotation = LerpKeyframe::GetValue<Quaternion>(GetRotations(), t, lerpType_);
	// 座標
	currentTransform_.translation = LerpKeyframe::GetValue<Vector3>(GetPositions(), t, lerpType_);

	// 任意値の更新
	UpdateAnyValues(t);
}

void KeyframeObject3D::UpdateKey() {

	// 補間中でキーの更新を許可していなければ何もしない
	if (!isUpdateKeyDuringLerp_ && currentState_ == State::Updating) {

		// 線の描画はする
		DrawKeyLine();
		return;
	}

	// トランスフォームに変更があれば更新
	for (size_t i = 0; i < keyObjects_.size(); ++i) {

		// 座標を比較して変更があれば更新
		const Transform3D& transform = keyObjects_[i]->GetTransform();
		if (transform.GetWorldScale() != keys_[i].transform.scale ||
			transform.GetWorldRotation() != keys_[i].transform.rotation ||
			transform.GetWorldPos() != keys_[i].transform.translation) {

			// トランスフォームを更新
			keys_[i].transform.scale = transform.GetWorldScale();
			keys_[i].transform.rotation = transform.GetWorldRotation();
			keys_[i].transform.translation = transform.GetWorldPos();
		}
	}

	// 線の描画
	DrawKeyLine();
}

std::unique_ptr<GameObject3D> KeyframeObject3D::CreateKeyObject(const Transform3D& transform) {

	// 生成
	std::unique_ptr<GameObject3D> object = std::make_unique<GameObject3D>();
	object->Init(keyModelName_, keyObjectName_, keyGroupName_);

	// 座標を設定
	object->SetScale(transform.scale);
	object->SetRotation(transform.rotation);
	object->SetTranslation(transform.translation);

	// 親がいれば親を設定
	if (!parentName_.empty()) {

		object->SetParent(*parent_);
	}

	// 描画設定、シーンにしか表示しない
	object->SetMeshRenderView(MeshRenderView::Scene);
	object->SetScale(isDrawKeyframe_ ? Vector3::AnyInit(1.0f) : Vector3::AnyInit(0.01f));
	object->SetCastShadow(false);
	object->SetShadowRate(1.0f);

	return object;
}

std::vector<Vector3> KeyframeObject3D::GetScales() const {

	// スケールリストを取得
	std::vector<Vector3> scales;
	scales.reserve(keys_.size());
	for (const auto& key : keys_) {

		scales.emplace_back(key.transform.scale);
	}
	return scales;
}

std::vector<Quaternion> KeyframeObject3D::GetRotations() const {

	// 回転リストを取得
	std::vector<Quaternion> rotations;
	rotations.reserve(keys_.size());
	for (const auto& key : keys_) {

		Quaternion rotation = key.transform.rotation;
		rotations.emplace_back(rotation);
	}
	return rotations;
}

std::vector<Vector3> KeyframeObject3D::GetPositions() const {

	// 座標リストを取得
	std::vector<Vector3> positions;
	positions.reserve(keys_.size());
	for (const auto& key : keys_) {

		Vector3 pos = key.transform.translation;
		positions.emplace_back(pos);
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

void KeyframeObject3D::UpdateAnyValues(float currentT) {

	// 何も値がなければ何もしない
	if (anyTracks_.empty() || keys_.empty()) {
		currentAnyValues_.clear();
		return;
	}

	// 任意の型の値の数分全て補間
	const uint32_t trackCount = static_cast<uint32_t>(anyTracks_.size());
	currentAnyValues_.resize(trackCount);
	// 型の数だけループ
	for (uint32_t trackIndex = 0; trackIndex < trackCount; ++trackIndex) {

		// 型ごとに分岐して補間
		AnyMold type = anyTracks_[trackIndex].type;
		switch (type) {
		case AnyMold::Float: {

			currentAnyValues_[trackIndex] = GetLerpedAnyValue<float>(trackIndex, currentT);
			break;
		}
		case AnyMold::Vector2: {

			currentAnyValues_[trackIndex] = GetLerpedAnyValue<Vector2>(trackIndex, currentT);
			break;
		}
		case AnyMold::Vector3: {

			currentAnyValues_[trackIndex] = GetLerpedAnyValue<Vector3>(trackIndex, currentT);
			break;
		}
		case AnyMold::Color: {

			currentAnyValues_[trackIndex] = GetLerpedAnyValue<Color>(trackIndex, currentT);
			break;
		}
		}
	}
}

void KeyframeObject3D::UpdateStartAnyValues(float easedT) {

	// 何も値がなければ何もしない
	if (anyTracks_.empty() || keys_.empty()) {
		currentAnyValues_.clear();
		return;
	}

	// 任意の型の値の数分全て補間
	const uint32_t trackCount = static_cast<uint32_t>(anyTracks_.size());
	currentAnyValues_.resize(trackCount);
	// 型の数だけループ
	for (uint32_t trackIndex = 0; trackIndex < trackCount; ++trackIndex) {

		// 型ごとに分岐して補間
		AnyMold type = anyTracks_[trackIndex].type;
		switch (type) {
		case AnyMold::Float: {

			currentAnyValues_[trackIndex] = LerpKeyframe::Lerp(std::get<float>(runtime_.startAnyValues[trackIndex]),
				std::get<float>(keys_.front().anyValues[trackIndex]), easedT);
			break;
		}
		case AnyMold::Vector2: {

			currentAnyValues_[trackIndex] = LerpKeyframe::Lerp(std::get<Vector2>(runtime_.startAnyValues[trackIndex]),
				std::get<Vector2>(keys_.front().anyValues[trackIndex]), easedT);
			break;
		}
		case AnyMold::Vector3: {

			currentAnyValues_[trackIndex] = LerpKeyframe::Lerp(std::get<Vector3>(runtime_.startAnyValues[trackIndex]),
				std::get<Vector3>(keys_.front().anyValues[trackIndex]), easedT);
			break;
		}
		case AnyMold::Color: {

			currentAnyValues_[trackIndex] = LerpKeyframe::Lerp(std::get<Color>(runtime_.startAnyValues[trackIndex]),
				std::get<Color>(keys_.front().anyValues[trackIndex]), easedT);
			break;
		}
		}
	}
}

KeyframeObject3D::AnyValue KeyframeObject3D::MakeDefaultAnyValue(AnyMold mold) {

	switch (mold) {
	case AnyMold::Float: return 0.0f;
	case AnyMold::Vector2: return Vector2::AnyInit(0.0f);
	case AnyMold::Vector3: return Vector3::AnyInit(0.0f);
	case AnyMold::Color: return Color::White();
	}
	return AnyValue{};
}

void KeyframeObject3D::ImGui() {

	ImGui::PushItemWidth(200.0f);

	// エディター内で更新を呼びだす
	if (isEditUpdate_) {

		SelfUpdate();
	}

	ImGui::SeparatorText("Key Timer");

	if (!keys_.empty()) {

		ImGui::Text("timer: %.2f / %.2f", timer_, keys_.back().time);

		float total = (std::max)(keys_.back().time, std::numeric_limits<float>::epsilon());
		float progress = timer_ / total;
		ImGui::Text("progress: %.2f", progress);

		// キータイムラインの描画
		DrawKeyTimeline();

		// 任意の型の現在値
		for (const auto& track : anyTracks_) {

			ImGui::SeparatorText(track.name.c_str());
			switch (track.type) {
			case AnyMold::Float:
				if (auto* value = std::get_if<float>(&currentAnyValues_[&track - &anyTracks_[0]])) {

					ImGui::Text("Current Value: %.3f", *value);
				}
				break;
			case AnyMold::Vector2:
				if (auto* value = std::get_if<Vector2>(&currentAnyValues_[&track - &anyTracks_[0]])) {

					ImGui::Text("Current Value: (%.3f, %.3f)", value->x, value->y);
				}
				break;
			case AnyMold::Vector3:
				if (auto* value = std::get_if<Vector3>(&currentAnyValues_[&track - &anyTracks_[0]])) {

					ImGui::Text("Current Value: (%.3f, %.3f, %.3f)", value->x, value->y, value->z);
				}
				break;
			case AnyMold::Color:
				if (auto* value = std::get_if<Color>(&currentAnyValues_[&track - &anyTracks_[0]])) {

					ImGui::Text("Current Value: (R: %.3f, G: %.3f, B: %.3f, A: %.3f)", value->r, value->g, value->b, value->a);
				}
				break;
			}
		}
	}

	ImGui::SeparatorText("Config");

	ImGui::Checkbox("isEditUpdate", &isEditUpdate_);
	if (ImGui::Checkbox("isDrawKeyframe", &isDrawKeyframe_)) {

		// キーオブジェクトの描画設定を更新
		for (const auto& keyObject : keyObjects_) {

			keyObject->SetScale(isDrawKeyframe_ ? Vector3::AnyInit(1.0f) : Vector3::AnyInit(0.01f));
		}
	}

	// キーオブジェクトの追加
	ImGui::DragFloat("addKeyTimeStep", &addKeyTimeStep_, 0.001f);
	if (ImGui::Button("Add Keyframe")) {

		// キーを追加
		Key key{};

		// 時間の初期化設定
		if (keys_.empty()) {

			key.time = 0.0f;
		} else {

			// 一番最後の時間から+設定値
			key.time = keys_.back().time + addKeyTimeStep_;
		}

		// 座標
		key.transform.translation = keyObjects_.empty() ?
			Vector3::AnyInit(0.0f) : keyObjects_.back()->GetTransform().GetWorldPos();
		key.transform.translation.y += 4.0f;
		// スケール
		key.transform.scale = keyObjects_.empty() ?
			Vector3::AnyInit(1.0f) : keyObjects_.back()->GetTransform().scale;
		// 回転
		key.transform.rotation = keyObjects_.empty() ?
			Quaternion::Identity() : keyObjects_.back()->GetTransform().rotation;

		// 任意の型の値があれば
		if (!anyTracks_.empty()) {
			if (keys_.empty()) {

				// 最初のキーなら、各任意値ごとにデフォルト値を入れる
				key.anyValues.reserve(anyTracks_.size());
				for (const auto& track : anyTracks_) {

					key.anyValues.emplace_back(MakeDefaultAnyValue(track.type));
				}
			} else {

				// 2個目以降のキーは直前のキーからコピーした値
				key.anyValues = keys_.back().anyValues;
			}
		}

		// キーを追加
		keys_.emplace_back(key);

		// キーオブジェクトを生成
		Transform3D initTransform;
		initTransform.Init();
		keyObjects_.emplace_back(std::move(CreateKeyObject(
			keys_.empty() ? initTransform : keys_.back().transform)));
	}
	// 開始
	if (ImGui::Button("Start")) {

		StartLerp();
	}

	if (ImGui::CollapsingHeader("Parameter")) {

		ImGui::SeparatorText("If Has Start");

		ImGui::DragFloat("startDuration", &startDuration_, 0.01f, 0.0f);
		EnumAdapter<EasingType>::Combo("startEaseType", &startEaseType_);

		ImGui::SeparatorText("Keys");

		ImGui::Checkbox("isUpdateKeyDuringLerp", &isUpdateKeyDuringLerp_);
		EnumAdapter<LerpKeyframe::Type>::Combo("LerpType", &lerpType_);

		if (!keys_.empty() && !anyTracks_.empty()) {
			// 任意値編集
			for (size_t track = 0; track < anyTracks_.size(); ++track) {

				ImGui::SeparatorText(anyTracks_[track].name.c_str());
				for (size_t k = 0; k < keys_.size(); ++k) {

					ImGui::PushID(static_cast<int>(k));
					std::string label = "Key " + std::to_string(k);

					// 型ごとに分岐して表示
					switch (anyTracks_[track].type) {
					case AnyMold::Float: {
						if (auto* value = std::get_if<float>(&keys_[k].anyValues[track])) {

							ImGuiHelper::DragFloat<float>(label.c_str(), *value);
						}
						break;
					}
					case AnyMold::Vector2: {
						if (auto* value = std::get_if<Vector2>(&keys_[k].anyValues[track])) {

							ImGuiHelper::DragFloat<Vector2>(label.c_str(), *value);
						}
						break;
					}
					case AnyMold::Vector3: {
						if (auto* value = std::get_if<Vector3>(&keys_[k].anyValues[track])) {

							ImGuiHelper::DragFloat<Vector3>(label.c_str(), *value);
						}
						break;
					}
					case AnyMold::Color: {
						if (auto* value = std::get_if<Color>(&keys_[k].anyValues[track])) {

							ImGuiHelper::DragFloat<Color>(label.c_str(), *value);
						}
						break;
					}
					}
					ImGui::PopID();
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("Set Parent")) {

		// 親子付けの解除
		if (ImGui::Button("Remove Parent")) {

			// キーオブジェクトの親を削除
			parentName_.clear();
			parent_ = nullptr;
			for (const auto& keyObject : keyObjects_) {

				keyObject->SetParent(Transform3D(), true);
			}
		}

		ImGui::Separator();

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

#if defined(_DEBUG) || defined(_DEVELOPBUILD)

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
	static int32_t s_dragIndex = -1;
	static bool s_dragging = false;
	static int32_t s_easeSeg = -1;

	// 先に丸のヒット＆ドラッグ処理
	ImVec2 mouse = ImGui::GetIO().MousePos;
	bool anyHovered = false;

	for (int32_t i = 0; i < static_cast<int32_t>(keys_.size()); ++i) {

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
		int32_t seg = -1;
		if (keys_.size() >= 2) {
			for (int32_t i = 0; i + 1 < static_cast<int32_t>(keys_.size()); ++i) {

				float a = std::clamp(keys_[i].time / total, 0.0f, 1.0f);
				float b = std::clamp(keys_[i + 1].time / total, 0.0f, 1.0f);
				if (a <= u && u <= b) {
					seg = i;
					break;
				}
			}
			if (seg < 0) {

				seg = static_cast<int32_t>(keys_.size()) - 2;
			}
		}
		s_easeSeg = seg;
		ImGui::OpenPopup("EasePopup");
	}

	// ポップアップでイージング選択
	if (ImGui::BeginPopup("EasePopup")) {
		if (0 <= s_easeSeg && s_easeSeg + 1 < static_cast<int32_t>(keys_.size())) {

			ImGui::Text("Segment: %d -> %d", s_easeSeg, s_easeSeg + 1);
			Easing::SelectEasingType(keys_[s_easeSeg].easeType);
			ImGui::Separator();
		}
		if (ImGui::Button("Close")) {

			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
#endif
}

void KeyframeObject3D::DrawKeyLine() {
#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	// 描画フラグがfalseなら描画しない
	if (!isDrawKeyframe_) {
		return;
	}

	// 線描画
	LerpKeyframe::DrawKeyframeLine(GetPositions(), lerpType_, isConnectEnds_);

	Color obbColor = Color::Yellow();
	// 色が任意で設定されていればその色にする
	for (const auto& anyValue : currentAnyValues_) {
		if (auto* color = std::get_if<Color>(&anyValue)) {

			obbColor = *color;
			break;
		}
	}
	// 表示オブジェクトも対応する色にする
	for (uint32_t index = 0; index < keys_.size(); ++index) {
		for (const auto& anyValue : keys_[index].anyValues) {
			if (auto* color = std::get_if<Color>(&anyValue)) {

				keyObjects_[index]->SetColor(*color);
			}
		}
	}

	// 現在の時間の点の位置
	LineRenderer::GetInstance()->DrawOBB(currentTransform_.translation,
		currentTransform_.scale, currentTransform_.rotation, obbColor, LineType::DepthIgnore);
#endif
}

void KeyframeObject3D::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	// キーをクリア
	keys_.clear();
	keyObjects_.clear();

	// キー位置を取得
	for (const auto& keyJson : data["Keys"]) {

		Key key{};

		// transformキーがなければ初期化させる
		if (keyJson.contains("Transform")) {

			key.transform.FromJson(keyJson["Transform"]);
		} else {

			key.transform.Init();
		}
		key.time = keyJson.value("time", 0.0f);
		key.easeType = EnumAdapter<EasingType>::FromString(keyJson.value("ease", "Linear")).value();

		key.anyValues.clear();
		// anyTracks_はFromJson前にAddKeyValueで追加されている前提
		if (!anyTracks_.empty()) {

			Json anyJson = Json::object();
			if (keyJson.contains("anyValues")) {

				anyJson = keyJson["anyValues"];
			}
			for (const auto& track : anyTracks_) {

				const std::string& name = track.name;
				AnyValue value;
				// JSON にキーがあればその値を無ければデフォルトを入れる
				if (anyJson.contains(name)) {

					const Json& vJson = anyJson[name];
					switch (track.type) {
					case AnyMold::Float: {

						float v = 0.0f;
						if (vJson.is_number_float() || vJson.is_number_integer()) {

							v = vJson.get<float>();
						} else {

							v = std::get<float>(MakeDefaultAnyValue(AnyMold::Float));
						}
						value = v;
						break;
					}
					case AnyMold::Vector2: {
						value = Vector2::FromJson(vJson);
						break;
					}
					case AnyMold::Vector3: {
						value = Vector3::FromJson(vJson);
						break;
					}
					case AnyMold::Color: {
						value = Color::FromJson(vJson);
						break;
					}
					}
				} else {
					value = MakeDefaultAnyValue(track.type);
				}

				key.anyValues.emplace_back(value);
			}
		}
		keys_.emplace_back(key);
	}

	parentName_ = data.value("parentName_", "");
	lerpType_ = EnumAdapter<LerpKeyframe::Type>::FromString(data.value("lerpType_", "Linear")).value();
	isConnectEnds_ = data.value("isConnectEnds_", false);
	isUpdateKeyDuringLerp_ = data.value("isUpdateKeyDuringLerp_", true);
	startDuration_ = data.value("startDuration_", 0.0f);
	startEaseType_ = EnumAdapter<EasingType>::FromString(data.value("startEaseType_", "Linear")).value();

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

		keyObjects_.emplace_back(std::move(CreateKeyObject(key.transform)));
	}

	addKeyTimeStep_ = data.value("addKeyTimeStep_", 0.8f);
}

void KeyframeObject3D::ToJson(Json& data) {

	uint32_t index = 0;
	for (auto& key : keys_) {

		Json keyJson;

		// トランスフォームはキーオブジェクトのローカルトランスフォームで保存
		keyObjects_[index]->SaveTransform(keyJson);

		keyJson["time"] = key.time;
		keyJson["ease"] = EnumAdapter<EasingType>::ToString(key.easeType);

		if (!anyTracks_.empty()) {

			Json anyJson = Json::object();
			// anyTracks_と key.anyValuesのindexを対応させる
			for (size_t trackIndex = 0; trackIndex < anyTracks_.size(); ++trackIndex) {

				const AnyTrack& track = anyTracks_[trackIndex];
				const std::string& name = track.name;
				AnyValue value;
				if (trackIndex < key.anyValues.size()) {

					value = key.anyValues[trackIndex];
				} else {

					value = MakeDefaultAnyValue(track.type);
				}
				// 型ごとにJsonに変換
				switch (track.type) {
				case AnyMold::Float:
					if (auto* v = std::get_if<float>(&value)) {

						anyJson[name] = *v;
					} else {

						anyJson[name] = 0.0f;
					}
					break;

				case AnyMold::Vector2:
					if (auto* v = std::get_if<Vector2>(&value)) {

						anyJson[name] = v->ToJson();;
					} else {

						anyJson[name] = Vector2::AnyInit(0.0f).ToJson();
					}
					break;

				case AnyMold::Vector3:
					if (auto* v = std::get_if<Vector3>(&value)) {

						anyJson[name] = v->ToJson();
					} else {

						anyJson[name] = Vector3::AnyInit(0.0f).ToJson();
					}
					break;

				case AnyMold::Color:
					if (auto* v = std::get_if<Color>(&value)) {

						anyJson[name] = v->ToJson();
					} else {

						anyJson[name] = Color::White().ToJson();
					}
					break;
				}
			}
			// 何か入っていればキーに追加
			if (!anyJson.empty()) {
				keyJson["anyValues"] = anyJson;
			}
		}

		data["Keys"].emplace_back(keyJson);

		++index;
	}

	data["parentName_"] = parentName_;
	data["lerpType_"] = EnumAdapter<LerpKeyframe::Type>::ToString(lerpType_);
	data["isConnectEnds_"] = isConnectEnds_;
	data["isUpdateKeyDuringLerp_"] = isUpdateKeyDuringLerp_;
	data["startDuration_"] = startDuration_;
	data["startEaseType_"] = EnumAdapter<EasingType>::ToString(startEaseType_);

	data["addKeyTimeStep_"] = addKeyTimeStep_;
}