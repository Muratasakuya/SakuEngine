#include "Input.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Window/WinApp.h>
#include <Engine/Core/Debug/SpdLogger.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Config.h>

#pragma comment(lib,"dInput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib, "xinput.lib")

//============================================================================
//	Input classMethods
//============================================================================

namespace {

	const char* kMouseNames[3] = { "MouseLeft", "MouseRight", "MouseCenter" };

	std::string MakeCallerTag(const std::source_location& location, std::string_view label) {

		std::string_view fn = location.function_name();

		auto pos = fn.find(' ');
		if (pos != std::string_view::npos) fn.remove_prefix(pos + 1);

		constexpr std::string_view cdeclStr = "__cdecl ";
		if (fn.starts_with(cdeclStr)) fn.remove_prefix(cdeclStr.size());

		std::string tag;
		tag.reserve(fn.size() + label.size() + 1);
		tag.append(fn);
		tag.push_back(':');
		tag.append(label);
		return tag;
	}

	const char* ToDikName(uint8_t dik) {

		switch (dik) {
		case DIK_W:   return "DIK_W";
		case DIK_A:   return "DIK_A";
		case DIK_S:   return "DIK_S";
		case DIK_D:   return "DIK_D";
		case DIK_R:   return "DIK_R";
		case DIK_E:   return "DIK_E";
		case DIK_Q:   return "DIK_Q";
		case DIK_UP:   return "DIK_UP";
		case DIK_DOWN:   return "DIK_DOWN";
		case DIK_RIGHT:   return "DIK_RIGHT";
		case DIK_LEFT:   return "DIK_LEFT";
		case DIK_F1: return "DIK_F1";
		case DIK_F2: return "DIK_F2";
		case DIK_F3: return "DIK_F3";
		case DIK_F10: return "DIK_F10";
		case DIK_F11: return "DIK_F11";
		case DIK_RETURN:   return "DIK_RETURN";
		case DIK_SPACE: return "DIK_SPACE";
		case DIK_ESCAPE: return "DIK_ESCAPE";
		default:      break;
		}
		static char buf[8];
		std::snprintf(buf, sizeof(buf), "0x%02X", dik);
		return buf;
	}

	template<size_t N, class TAG_FUNC, class NAME_FUNC>
	void LogEnterStayExit(bool now, bool prev,
		size_t idx, std::array<std::chrono::steady_clock::time_point, N>& start,
		std::array<bool, N>& stayDone, TAG_FUNC&& makeTag, NAME_FUNC&& toName) {

		if (now) {
			if (!prev) {

				start[idx] = std::chrono::steady_clock::now();
				stayDone[idx] = false;

				LOG_INFO("{}  Enter {}", makeTag(), toName());
			} else if (!stayDone[idx]) {

				auto dur = std::chrono::steady_clock::now() - start[idx];

				LOG_INFO("{}  Stay  {}  {:.1f}ms",
					makeTag(), toName(),
					std::chrono::duration<float, std::milli>(dur).count());

				stayDone[idx] = true;
			}
		} else if (prev) {

			auto dur = std::chrono::steady_clock::now() - start[idx];

			LOG_INFO("{}  Exit  {}  {:.1f}ms",
				makeTag(), toName(),
				std::chrono::duration<float, std::milli>(dur).count());
		}
	}
}

Input* Input::instance_ = nullptr;

void Input::SetViewRect(InputViewArea viewArea, const Vector2& dstPos, Vector2 dstSize) {

	viewRects_[viewArea].dstPos = dstPos;
	viewRects_[viewArea].dstSize = dstSize;
}

bool Input::IsMouseOnView(InputViewArea viewArea) const {

	Vector2 mouse = GetMousePos();
	const ViewRect rect = viewRects_.at(viewArea);
	return (mouse.x >= rect.dstPos.x && mouse.y >= rect.dstPos.y &&
		mouse.x < rect.dstPos.x + rect.dstSize.x &&
		mouse.y < rect.dstPos.y + rect.dstSize.y);
}

std::optional<Vector2> Input::GetMousePosInView(InputViewArea viewArea) const {

	if (!IsMouseOnView(viewArea)) {
		return std::nullopt;
	}
	Vector2 mouse = GetMousePos();
	const ViewRect rect = viewRects_.at(viewArea);
	Vector2 local = Vector2(mouse.x - rect.dstPos.x, mouse.y - rect.dstPos.y);
	return local * (rect.srcSize / rect.dstSize);
}

Input* Input::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new Input();
	}
	return instance_;
}

void Input::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

bool Input::PushKey(BYTE keyNumber, const std::source_location& location) {

	const bool now = key_[keyNumber];
	const bool prev = keyPre_[keyNumber];

	if (now) {
		if (!prev) {
			keyStartTime_[keyNumber] = std::chrono::steady_clock::now();
			keyStayLogged_[keyNumber] = false;

			LOG_INFO("{}  Enter {}",
				MakeCallerTag(location, "Key"),
				ToDikName(keyNumber));
		} else if (!keyStayLogged_[keyNumber]) {

			auto dur = std::chrono::steady_clock::now() - keyStartTime_[keyNumber];

			LOG_INFO("{}  Stay  {}  {:.1f}ms",
				MakeCallerTag(location, "Key"),
				ToDikName(keyNumber),
				std::chrono::duration<float, std::milli>(dur).count());

			keyStayLogged_[keyNumber] = true;
		}
	} else if (prev) {

		auto dur = std::chrono::steady_clock::now() - keyStartTime_[keyNumber];

		LOG_INFO("{}  Exit  {}  {:.1f}ms",
			MakeCallerTag(location, "Key"),
			ToDikName(keyNumber),
			std::chrono::duration<float, std::milli>(dur).count());
	}

	return now;
}

bool Input::TriggerKey(BYTE keyNumber, const std::source_location& location) {

	// 現在のフレームで押されていて、前のフレームで押されていなかった場合にtrueを返す
	if (key_[keyNumber] && !keyPre_[keyNumber]) {

		LOG_INFO("{}", MakeCallerTag(location, "TriggerKey:" + std::string(ToDikName(keyNumber))));
		return true;
	}

	return false;
}
bool Input::ReleaseKey(BYTE keyNumber, const std::source_location& location) {

	if (!key_[keyNumber] && keyPre_[keyNumber]) {

		LOG_INFO("{}", MakeCallerTag(location, "ReleaseKey:" + std::string(ToDikName(keyNumber))));
		return true;
	}
	return false;
}
bool Input::PushGamepadButton(GamePadButtons button, const std::source_location& location) {

	const size_t index = static_cast<size_t>(button);
	const bool now = gamepadButtons_[index];
	const bool prev = gamepadButtonsPre_[index];

	LogEnterStayExit(now, prev, index,
		gpStartTime_, gpStayLogged_,
		[&] { return MakeCallerTag(location, "GamePadButtons:"); },
		[&] { return EnumAdapter<GamePadButtons>::ToString(button); });

	return now;
}
bool Input::TriggerGamepadButton(GamePadButtons button, const std::source_location& location) {

	bool trigger = gamepadButtons_[static_cast<size_t>(button)] && !gamepadButtonsPre_[static_cast<size_t>(button)];
	if (trigger) {

		LOG_INFO("{}", MakeCallerTag(location,
			"TriggerGamepadButton:" + std::string(EnumAdapter<GamePadButtons>::ToString(button))));
	}

	return trigger;
}
float Input::GetLeftTriggerValue() const {

	return leftTriggerValue_;
}
float Input::GetRightTriggerValue() const {

	return rightTriggerValue_;
}
Vector2 Input::GetLeftStickVal() const {
	return { leftThumbX_,leftThumbY_ };
}
Vector2 Input::GetRightStickVal() const {
	return { rightThumbX_,rightThumbY_ };
}
float Input::ApplyDeadZone(float value) {

	if (std::fabs(value) < deadZone_) {
		return 0.0f;
	}
	return value;
}
Vector2 Input::GetMousePos() const {

	return mousePos_;
}
Vector2 Input::GetMousePrePos() const {

	return mousePrePos_;
}
Vector2 Input::GetMouseMoveValue() const {

	return { static_cast<float>(mouseState_.lX),static_cast<float>(mouseState_.lY) };
}
float Input::GetMouseWheel() {

	return wheelValue_;
}
bool Input::PushMouseButton(size_t index, const std::source_location& location) const {

	const bool now = mouseButtons_[index];
	const bool prev = mousePreButtons_[index];

	auto& start = const_cast<Input*>(this)->mouseStartTime_;
	auto& stayDone = const_cast<Input*>(this)->mouseStayLogged_;

	LogEnterStayExit(now, prev, index,
		start, stayDone,
		[&] { return MakeCallerTag(location, "Mouse"); },
		[&] { return kMouseNames[index]; });

	return now;
}
bool Input::PushMouse(MouseButton button, const std::source_location& location) const {

	bool push = false;
	switch (button) {
	case MouseButton::Right: {

		push = PushMouseRight(location);
		break;
	}
	case MouseButton::Left: {

		push = PushMouseLeft(location);
		break;
	}
	case MouseButton::Center: {

		push = PushMouseCenter(location);
		break;
	}
	}
	return push;
}
bool Input::TriggerMouseLeft(const std::source_location& location) const {

	bool trigger = !mousePreButtons_[0] && mouseButtons_[0];
	if (trigger) {

		LOG_INFO("{}  mouseLeft=triggered", MakeCallerTag(location, "TriggerMouseLeft"));
	}

	return trigger;
}
bool Input::TriggerMouseRight(const std::source_location& location) const {

	bool trigger = !mousePreButtons_[1] && mouseButtons_[1];
	if (trigger) {

		LOG_INFO("{}  mouseRight=triggered", MakeCallerTag(location, "TriggerMouseRight"));
	}

	return trigger;
}
bool Input::TriggerMouseCenter(const std::source_location& location) const {

	bool trigger = !mousePreButtons_[2] && mouseButtons_[2];
	if (trigger) {

		LOG_INFO("{}  mouseCenter=triggered", MakeCallerTag(location, "TriggerMouseCenter"));
	}

	return trigger;
}
bool Input::TriggerMouse(MouseButton button, const std::source_location& location) const {

	bool trigger = false;
	switch (button) {
	case MouseButton::Right: {

		trigger = TriggerMouseRight(location);
		break;
	}
	case MouseButton::Left: {

		trigger = TriggerMouseLeft(location);
		break;
	}
	case MouseButton::Center: {

		trigger = TriggerMouseCenter(location);
		break;
	}
	}
	return trigger;
}
bool Input::ReleaseMouse(MouseButton button, const std::source_location& location) const {

	bool released = false;
	switch (button) {
	case MouseButton::Left: {

		released = !mouseButtons_[0] && mousePreButtons_[0];
		if (released) {

			LOG_INFO("{}  mouseLeft=released", MakeCallerTag(location, "ReleaseMouseLeft"));
		}
		break;
	}
	case MouseButton::Right: {

		released = !mouseButtons_[1] && mousePreButtons_[1];
		if (released) {

			LOG_INFO("{}  mouseRight=released", MakeCallerTag(location, "ReleaseMouseRight"));
		}
		break;
	}
	case MouseButton::Center: {

		released = !mouseButtons_[2] && mousePreButtons_[2];
		if (released) {

			LOG_INFO("{}  mouseCenter=released", MakeCallerTag(location, "ReleaseMouseCenter"));
		}
		break;
	}
	}
	return released;
}
void Input::SetDeadZone(float deadZone) {

	deadZone_ = deadZone;
}

void Input::Init(WinApp* winApp) {

	winApp_ = winApp;

	HRESULT hr;

	// DirectInputの初期化
	dInput_ = nullptr;
	hr = DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dInput_, nullptr);
	assert(SUCCEEDED(hr));

	// キーボードデバイスの初期化
	keyboard_ = nullptr;
	hr = dInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(hr));

	// 入力データ形式のセット 標準形式
	hr = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));

	// 排他制御レベルのリセット
	hr = keyboard_->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	// マウスデバイスの初期化
	hr = dInput_->CreateDevice(GUID_SysMouse, &mouse_, NULL);
	assert(SUCCEEDED(hr));

	// 入力データ形式のセット
	hr = mouse_->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(hr));

	// 排他制御レベルのリセット
	hr = mouse_->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(hr));

	// マウスの取得開始
	hr = mouse_->Acquire();

	// 初期化値
	viewRects_[InputViewArea::Game].dstPos = Vector2::AnyInit(0.0f);
	viewRects_[InputViewArea::Game].dstSize = Vector2::AnyInit(0.0f);
	viewRects_[InputViewArea::Game].srcSize = Vector2(Config::kWindowWidthf, Config::kWindowHeightf);
	viewRects_[InputViewArea::Scene].dstPos = Vector2::AnyInit(0.0f);
	viewRects_[InputViewArea::Scene].dstSize = Vector2::AnyInit(0.0f);
	viewRects_[InputViewArea::Scene].srcSize = Vector2(Config::kWindowWidthf, Config::kWindowHeightf);
}

void Input::Update() {

	mousePrePos_ = mousePos_;
	mousePreButtons_ = mouseButtons_;

	HRESULT hr;

	// キーボード情報の取得開始
	hr = keyboard_->Acquire();

	// 前回のキー入力を保存
	std::memcpy(keyPre_.data(), key_.data(), key_.size());

	// 全キーの入力状態を取得する
	hr = keyboard_->GetDeviceState(static_cast<DWORD>(key_.size()), key_.data());

	// 前回のゲームパッドの状態を保存
	gamepadStatePre_ = gamepadState_;
	std::memcpy(gamepadButtonsPre_.data(), gamepadButtons_.data(), gamepadButtons_.size());

	// ゲームパッドの現在の状態を取得
	ZeroMemory(&gamepadState_, sizeof(XINPUT_STATE));
	DWORD dwResult = XInputGetState(0, &gamepadState_);

	for (const auto& key : key_) {
		if (key) {

			inputType_ = InputType::Keyboard;
			break;
		}
	}
	// マウス入力があれば
	if (mouseButtons_[0] || mouseButtons_[1] || mouseButtons_[2] || GetMouseMoveValue().Length() != 0.0f) {
		inputType_ = InputType::Keyboard;
	}

	if (dwResult == ERROR_SUCCESS) {
		for (const auto& button : gamepadButtons_) {
			if (button) {

				inputType_ = InputType::GamePad;
				break;
			}
		}

		// スティック入力があれば
		if (GetLeftStickVal().Length() > deadZone_ || GetRightStickVal().Length() > deadZone_) {
			inputType_ = InputType::GamePad;
		}
	}

	if (dwResult == ERROR_SUCCESS) {

#pragma region ///ゲームパッドが接続されている場合の処理 ///
		gamepadButtons_[static_cast<size_t>(GamePadButtons::ARROW_UP)] = (gamepadState_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
		gamepadButtons_[static_cast<size_t>(GamePadButtons::ARROW_DOWN)] = (gamepadState_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
		gamepadButtons_[static_cast<size_t>(GamePadButtons::ARROW_LEFT)] = (gamepadState_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
		gamepadButtons_[static_cast<size_t>(GamePadButtons::ARROW_RIGHT)] = (gamepadState_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
		gamepadButtons_[static_cast<size_t>(GamePadButtons::START)] = (gamepadState_.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
		gamepadButtons_[static_cast<size_t>(GamePadButtons::BACK)] = (gamepadState_.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;
		gamepadButtons_[static_cast<size_t>(GamePadButtons::LEFT_THUMB)] = (gamepadState_.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
		gamepadButtons_[static_cast<size_t>(GamePadButtons::RIGHT_THUMB)] = (gamepadState_.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
		gamepadButtons_[static_cast<size_t>(GamePadButtons::LEFT_SHOULDER)] = (gamepadState_.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
		gamepadButtons_[static_cast<size_t>(GamePadButtons::RIGHT_SHOULDER)] = (gamepadState_.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
		gamepadButtons_[static_cast<size_t>(GamePadButtons::LEFT_TRIGGER)] = (leftTriggerValue_ > 0);
		gamepadButtons_[static_cast<size_t>(GamePadButtons::RIGHT_TRIGGER)] = (rightTriggerValue_ > 0);
		gamepadButtons_[static_cast<size_t>(GamePadButtons::A)] = (gamepadState_.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
		gamepadButtons_[static_cast<size_t>(GamePadButtons::B)] = (gamepadState_.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
		gamepadButtons_[static_cast<size_t>(GamePadButtons::X)] = (gamepadState_.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
		gamepadButtons_[static_cast<size_t>(GamePadButtons::Y)] = (gamepadState_.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;

		// スティックの状態を更新
		leftThumbX_ = ApplyDeadZone(gamepadState_.Gamepad.sThumbLX);
		leftThumbY_ = ApplyDeadZone(gamepadState_.Gamepad.sThumbLY);
		rightThumbX_ = ApplyDeadZone(gamepadState_.Gamepad.sThumbRX);
		rightThumbY_ = ApplyDeadZone(gamepadState_.Gamepad.sThumbRY);

		leftTriggerValue_ = static_cast<float>(gamepadState_.Gamepad.bLeftTrigger) / 255.0f;
		rightTriggerValue_ = static_cast<float>(gamepadState_.Gamepad.bRightTrigger) / 255.0f;
#pragma endregion
	} else {

		// ゲームパッドが接続されていない場合の処理
		std::fill(gamepadButtons_.begin(), gamepadButtons_.end(), false);

		leftThumbX_ = 0.0f;
		leftThumbY_ = 0.0f;
		rightThumbX_ = 0.0f;
		rightThumbY_ = 0.0f;

		leftTriggerValue_ = 0.0f;
		rightTriggerValue_ = 0.0f;
	}

	// マウス情報の取得開始
	hr = mouse_->Acquire();
	if (FAILED(hr)) {
		if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {

			mouse_->Acquire();
		}
	}

	hr = mouse_->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState_);

	if (FAILED(hr)) {
		// 取得失敗時の処理
		std::fill(mouseButtons_.begin(), mouseButtons_.end(), false);
		mousePos_ = { 0.0f, 0.0f };
	} else {

		// マウスボタンの状態を保存
		mouseButtons_[0] = (mouseState_.rgbButtons[0] & 0x80) != 0;
		mouseButtons_[1] = (mouseState_.rgbButtons[1] & 0x80) != 0;
		mouseButtons_[2] = (mouseState_.rgbButtons[2] & 0x80) != 0;

		POINT point;
		GetCursorPos(&point);
		ScreenToClient(winApp_->GetHwnd(), &point);

		// マウスの移動量を保存
		mousePos_.x = static_cast<float>(point.x);
		mousePos_.y = static_cast<float>(point.y);

		// ホイール値
		wheelValue_ = static_cast<float>(mouseState_.lZ) / WHEEL_DELTA;
	}
}