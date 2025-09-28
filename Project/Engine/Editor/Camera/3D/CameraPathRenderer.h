#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Camera/3D/CameraPathData.h>

//============================================================================
//	CameraPathRenderer class
//============================================================================
class CameraPathRenderer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	CameraPathRenderer() = default;
	~CameraPathRenderer() = default;

	void DrawLine3D(const CameraPathData& data) const;
};