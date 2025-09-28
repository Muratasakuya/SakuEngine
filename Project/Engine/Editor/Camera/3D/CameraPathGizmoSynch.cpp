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
		if (keyframe.demoObject && keyframe.demoObject->GetObjectID() == *selected) {

			return i;
		}
	}
	return selectedKeyIndex;
}

void CameraPathGizmoSynch::UpdateFollowTarget(CameraPathData& data) const {

	ImGuiObjectEditor* objectEditor = ImGuiObjectEditor::GetInstance();
	const bool usingGizmo = objectEditor->IsUsingGuizmo();
	const std::optional<uint32_t> selectId = objectEditor->GetSelected3D();

	// 追従先があるならオフセットを更新する
	for (auto& keyframe : data.keyframes) {

		// falseの場合は常に0.0f
		if (!(data.followTarget && data.target)) {

			keyframe.demoObject->SetOffsetTranslation(Vector3::AnyInit(0.0f));
			continue;
		}

		// 追従先のワールド座標に設定
		keyframe.demoObject->SetOffsetTranslation(data.target->GetWorldPos());

		// ギズモ操作中は更新できないようにする
		if (usingGizmo && selectId && (*selectId == keyframe.demoObject->GetObjectID())) {

			// 回転前のローカル座標を設定
			const Quaternion inverse = Quaternion::Conjugate(data.target->rotation);
			keyframe.translation = inverse * keyframe.demoObject->GetTranslation();
			continue;
		}
		// 回転を考慮した座標を設定
		keyframe.demoObject->SetTranslation(data.target->rotation * keyframe.translation);
	}
}