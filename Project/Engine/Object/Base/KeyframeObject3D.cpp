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
}

void KeyframeObject3D::StartLerp() {

	// 補間開始
	currentState_ = State::Updating;
	timer_.Reset();
}

void KeyframeObject3D::Update() {

	// None状態なら何もしない
	if (currentState_ == State::None) {
		return;
	}

	// 時間を更新
	timer_.Update();

	// 現在の補間位置を更新
	currentPos_ = LerpKeyframe::GetValue<Vector3>(keyPositions_, timer_.easedT_, lerpType_);

	// 時間経過で終了
	if (timer_.IsReached()) {

		// 最終位置を設定
		currentPos_ = keyPositions_.back();

		// リセット
		currentState_ = State::None;
		timer_.Reset();
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
	object->SetMeshRenderView(MeshRenderView::Scene);
	object->SetBlendMode(BlendMode::kBlendModeAdd);
	object->SetScale(Vector3::AnyInit(2.4f));
	object->SetCastShadow(false);
	object->SetShadowRate(1.0f);

	return object;
}

void KeyframeObject3D::ImGui() {

	// エディター内で更新を呼びだす
	if (isEditUpdate_) {

		Update();
	}

	ImGui::Checkbox("isEditUpdate", &isEditUpdate_);

	// キーオブジェクトの追加
	if (ImGui::Button("Add Keyframe")) {

		// キーオブジェクトを生成
		keyObjects_.emplace_back(std::move(CreateKeyObject(
			keyPositions_.empty() ? Vector3(0.0f, 16.0f, 0.0f) : keyPositions_.back())));

		// キー位置を追加
		// 最後のキー位置をコピー
		Vector3 newPos = keyObjects_.back()->GetTransform().GetWorldPos();
		newPos.y += 4.0f;
		keyPositions_.emplace_back(newPos);
	}
	// 開始
	if (ImGui::Button("Start")) {

		StartLerp();
	}

	if (ImGui::CollapsingHeader("Parameter")) {

		ImGui::Checkbox("isConnectEnds", &isConnectEnds_);
		EnumAdapter<LerpKeyframe::Type>::Combo("LerpType", &lerpType_);

		timer_.ImGui("LerpTime");
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
		if (worldPos != keyPositions_[i]) {

			// 座標を更新
			keyPositions_[i] = worldPos;
		}
	}
	// 線描画
	LerpKeyframe::DrawKeyframeLine(keyPositions_, lerpType_, isConnectEnds_);
	// 現在の時間の点の位置
	LineRenderer::GetInstance()->DrawSphere(6, 4.0f, currentPos_, Color::Yellow());
}

void KeyframeObject3D::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	// キー位置を取得
	for (const auto& keyPos : data["KeyPositions"]) {

		keyPositions_.emplace_back(Vector3::FromJson(keyPos));
	}
	parentName_ = data.value("parentName_", "");
	lerpType_ = EnumAdapter<LerpKeyframe::Type>::FromString(data.value("lerpType_", "Linear")).value();
	isConnectEnds_ = data.value("isConnectEnds_", false);
	timer_.FromJson(data["Timer"]);

	// キーオブジェクトを生成
	for (const auto& pos : keyPositions_) {

		keyObjects_.emplace_back(std::move(CreateKeyObject(pos)));
	}
}

void KeyframeObject3D::ToJson(Json& data) {

	for (const auto& keyObject : keyObjects_) {

		data["KeyPositions"].emplace_back(keyObject->GetTranslation().ToJson());
	}
	data["parentName_"] = parentName_;
	data["lerpType_"] = EnumAdapter<LerpKeyframe::Type>::ToString(lerpType_);
	data["isConnectEnds_"] = isConnectEnds_;
	timer_.ToJson(data["Timer"]);
}