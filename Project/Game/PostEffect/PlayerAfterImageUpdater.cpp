#include "PlayerAfterImageUpdater.h"

//============================================================================
//	PlayerAfterImageUpdater classMethods
//============================================================================

void PlayerAfterImageUpdater::Init() {

	// 初期値設定
	bufferData_.rate = 0.0f;

	// json適応
	ApplyJson();
}

void PlayerAfterImageUpdater::Update() {
}

void PlayerAfterImageUpdater::Start(const Color& color) {

	// レートを1.0fにしてディザリング処理させる
	bufferData_.rate = 1.0f;
	bufferData_.ditherColor = color;
	bufferData_.emissionColor = Vector3(color.r, color.g, color.b);
}

void PlayerAfterImageUpdater::Reset() {

	// レートを0.0fにしてディザリング処理させない
	bufferData_.rate = 0.0f;
}

void PlayerAfterImageUpdater::ImGui() {

	SaveButton();
	ImGui::Separator();

	ImGui::PushID("PlayerAfterImageUpdater");

	ImGui::DragFloat("rate", &bufferData_.rate, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("divisor", &bufferData_.divisor, 0.01f, 0.01f, 4.0f);
	ImGui::ColorEdit4("ditherColot", &bufferData_.ditherColor.r);
	ImGui::ColorEdit3("emissionColor", &bufferData_.emissionColor.x);
	ImGui::DragFloat("emissionIntensity", &bufferData_.emissionIntensity, 0.01f);

	ImGui::PopID();
}

void PlayerAfterImageUpdater::ApplyJson() {

	Json data;
	if (!LoadFile(data)) {
		return;
	}

	bufferData_.divisor = data.value("divisor", 4.0f);
	bufferData_.ditherColor = Color::FromJson(data.value("ditherColor", Json()));
	bufferData_.emissionColor = Vector3::FromJson(data.value("emissionColor", Json()));
	bufferData_.emissionIntensity = data.value("emissionIntensity", 1.0f);
}

void PlayerAfterImageUpdater::SaveJson() {

	Json data;

	data["divisor"] = bufferData_.divisor;
	data["ditherColor"] = bufferData_.ditherColor.ToJson();
	data["emissionColor"] = bufferData_.emissionColor.ToJson();
	data["emissionIntensity"] = bufferData_.emissionIntensity;

	SaveFile(data);
}