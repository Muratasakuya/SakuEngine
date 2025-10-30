#include "CRTDisplayUpdater.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Timer/GameTimer.h>

//============================================================================
//	CRTDisplayUpdater classMethods
//============================================================================

void CRTDisplayUpdater::Init() {

	// json適応
	ApplyJson();
}

void CRTDisplayUpdater::Start() {
}

void CRTDisplayUpdater::Update() {

	// 時間の更新
	bufferData_.time += GameTimer::GetScaledDeltaTime();
}

void CRTDisplayUpdater::ImGui() {

	SaveButton();
	ImGui::Separator();

	ImGui::PushItemWidth(192.0f);

	EnumAdapter<State>::Combo("currentState", &currentState_);

	bufferData_.ImGui();

	ImGui::PopItemWidth();
}

void CRTDisplayUpdater::ApplyJson() {

	Json data;
	if (!LoadFile(data)) {
		return;
	}

	bufferData_.resolution = Vector2::FromJson(data.value("resolution", Json()));
	bufferData_.barrel = data.value("barrel", bufferData_.barrel);
	bufferData_.zoom = data.value("zoom", bufferData_.zoom);
	bufferData_.cornerRadius = data.value("cornerRadius", bufferData_.cornerRadius);
	bufferData_.cornerFeather = data.value("cornerFeather", bufferData_.cornerFeather);
	bufferData_.vignette = data.value("vignette", bufferData_.vignette);
	bufferData_.aberration = data.value("aberration", bufferData_.aberration);
	bufferData_.scanlineIntensity = data.value("scanlineIntensity", bufferData_.scanlineIntensity);
	bufferData_.scanlineCount = data.value("scanlineCount", bufferData_.scanlineCount);
	bufferData_.noise = data.value("noise", bufferData_.noise);
}

void CRTDisplayUpdater::SaveJson() {

	Json data;

	data["resolution"] = bufferData_.resolution.ToJson();
	data["barrel"] = bufferData_.barrel;
	data["zoom"] = bufferData_.zoom;
	data["cornerRadius"] = bufferData_.cornerRadius;
	data["cornerFeather"] = bufferData_.cornerFeather;
	data["vignette"] = bufferData_.vignette;
	data["aberration"] = bufferData_.aberration;
	data["scanlineIntensity"] = bufferData_.scanlineIntensity;
	data["scanlineCount"] = bufferData_.scanlineCount;
	data["noise"] = bufferData_.noise;

	SaveFile(data);
}