#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Camera/3D/CameraPathData.h>

//============================================================================
//	CameraPathGizmoSynch class
//============================================================================
class CameraPathGizmoSynch {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	CameraPathGizmoSynch() = default;
	~CameraPathGizmoSynch() = default;

	// 選択されたオブジェクトの更新
	int SynchSelectedKeyIndex(const CameraPathData& data, int selectedKeyIndex) const;

	// 追従先のオフセットの更新
	void UpdateFollowTarget(CameraPathData& data) const;

	// ローカルからワールド位置に変換
	void ApplyLocalToWorldByTarget(CameraPathData& data) const;
};