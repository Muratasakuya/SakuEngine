#include "CameraPathGizmoSynch.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>

//============================================================================
//	CameraPathGizmoSynch classMethods
//============================================================================

int CameraPathGizmoSynch::SynchSelectedKeyIndex(const CameraPathData& data, int selectedKeyIndex) const {

	// 選択されているオブジェクトに合わせる
	auto selected = ImGuiObjectEditor::GetInstance()->GetSelected3D();
	if (!selected) {
		return selectedKeyIndex;
	}
	const auto& keyframes = data.keyframes;
	for (int i = 0; i < static_cast<int>(keyframes.size()); ++i) {

		const auto& keyframe = keyframes[i];
		keyframe.demoObject->SetColor(Color::White());
		if (keyframe.demoObject->GetObjectID() == *selected) {
			keyframe.demoObject->SetColor(Color::Red());
			return i;
		}
	}
	return selectedKeyIndex;
}

void CameraPathGizmoSynch::UpdateFollowTarget(CameraPathData& data) const {

	ImGuiObjectEditor* objectEditor = ImGuiObjectEditor::GetInstance();
	const std::optional<uint32_t> selectId = objectEditor->GetSelected3D();

	// 追従先があるならオフセットを更新する
	for (auto& keyframe : data.keyframes) {

		// 追従先がない場合はデフォルトを設定
		if (!(data.followTarget && data.target)) {
			keyframe.demoObject->SetOffsetTranslation(Vector3::AnyInit(0.0f));
			keyframe.demoObject->SetRotation(keyframe.rotation);
			continue;
		}

		// ワールド座標でオフセットを設定
		keyframe.demoObject->SetOffsetTranslation(data.target->GetWorldPos());

		// ローカル回転を設定
		const Quaternion targetRotation = Quaternion::Normalize(data.target->rotation);
		const Quaternion inverseTarget = Quaternion::Conjugate(targetRotation);

		const Vector3 worldTranslate = keyframe.demoObject->GetTranslation();
		const Quaternion worldRotation = keyframe.demoObject->GetRotation();

		// ターゲット空間に変換
		keyframe.translation = inverseTarget * worldTranslate;
		if (data.followRotation) {

			keyframe.rotation = Quaternion::Normalize(inverseTarget * worldRotation);
		} else {

			keyframe.rotation = worldRotation;
		}
		keyframe.demoObject->SetTranslation(targetRotation * keyframe.translation);
		keyframe.demoObject->SetRotation(data.followRotation ?
			Quaternion::Normalize(targetRotation * keyframe.rotation) :
			keyframe.rotation);
	}
}

void CameraPathGizmoSynch::ApplyLocalToWorldByTarget(CameraPathData& data) const {

	// 追従先がいるかどうか
	const bool hasTarget = (data.followTarget && data.target);
	const Vector3 targetTranslation = hasTarget ? data.target->GetWorldPos()
		: Vector3::AnyInit(0.0f);
	const Quaternion targetRotation = hasTarget ?
		Quaternion::Normalize(data.target->rotation)
		: Quaternion::Identity();
	for (const auto& keyframe : data.keyframes) {

		// 0回転時のローカルをワールドに変換
		keyframe.demoObject->SetOffsetTranslation(targetTranslation);
		keyframe.demoObject->SetTranslation(targetRotation * keyframe.translation);
		keyframe.demoObject->SetRotation(data.followRotation ?
			Quaternion::Normalize(targetRotation * keyframe.rotation) :
			keyframe.rotation);
	}
}