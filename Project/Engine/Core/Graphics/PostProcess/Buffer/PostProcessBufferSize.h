#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/MathLib/MathUtils.h>

//============================================================================
//	DerivedBuffer class
//	GPU転送データ、ImGui関数で値の調整ができる
//============================================================================

class GrayscaleForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GrayscaleForGPU() = default;
	~GrayscaleForGPU() = default;

	void ImGui();

	//--------- properties ---------------------------------------------------

	// 1.0fが最大のグレースケール割合
	float rate = 1.0f;

	float pad[3];
};

class DissolveForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DissolveForGPU() = default;
	~DissolveForGPU() = default;

	void ImGui();

	//--------- properties ---------------------------------------------------

	float threshold;
	float edgeSize;
	float padding1[2];
	Vector3 edgeColor;
	float padding2;
	Vector3 thresholdColor;
	float padding3;
};

class BloomForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BloomForGPU() = default;
	~BloomForGPU() = default;

	void ImGui();

	//--------- properties ---------------------------------------------------

	float threshold = 1.0f;
	int radius = 8;
	float sigma = 64.0f;
};

class GaussianFilterForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GaussianFilterForGPU() = default;
	~GaussianFilterForGPU() = default;

	void ImGui();

	//--------- properties ---------------------------------------------------

	float sigma = 1.0f;
};

class LuminanceBasedOutlineForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	LuminanceBasedOutlineForGPU() = default;
	~LuminanceBasedOutlineForGPU() = default;

	void ImGui();

	//--------- properties ---------------------------------------------------

	float strength;
};

class DepthBasedOutlineForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DepthBasedOutlineForGPU() = default;
	~DepthBasedOutlineForGPU() = default;

	void ImGui();

	void SetProjectionInverse(const Matrix4x4& projection) { projectionInverse = projection; }

	//--------- properties ---------------------------------------------------

	Matrix4x4 projectionInverse;

	float edgeScale = 1.0f;
	float threshold = 12.0f;
	float pad0[2];

	Vector3 color = Vector3::AnyInit(0.0f);
	float pad1;
};

class VignetteForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	VignetteForGPU() = default;
	~VignetteForGPU() = default;

	void ImGui();

	//--------- properties ---------------------------------------------------

	float scale = 24.0f;
	float power = 0.0f;
	float padding[2];
	Vector3 color = Vector3::AnyInit(1.0f);
};

class RadialBlurForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	RadialBlurForGPU() = default;
	~RadialBlurForGPU() = default;

	void ImGui();

	//--------- properties ---------------------------------------------------

	Vector2 center = Vector2::AnyInit(0.5f);
	int numSamples = 0;
	float width = 0.0f;
};

class HorizonBlurForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	HorizonBlurForGPU() = default;
	~HorizonBlurForGPU() = default;

	void ImGui();

	void ImGuiWithBloom();

	//--------- properties ---------------------------------------------------

	int radius;
	float sigma;
};

class VerticalBlurForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	VerticalBlurForGPU() = default;
	~VerticalBlurForGPU() = default;

	void ImGui();

	void ImGuiWithBloom();

	//--------- properties ---------------------------------------------------

	int radius;
	float sigma;
};

class LuminanceExtractForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	LuminanceExtractForGPU() = default;
	~LuminanceExtractForGPU() = default;

	void ImGui();

	void ImGuiWithBloom();

	//--------- properties ---------------------------------------------------

	float threshold;
};

class LutForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	LutForGPU() = default;
	~LutForGPU() = default;

	void ImGui();

	//--------- properties ---------------------------------------------------

	float lerpRate = 0.2f;
	float lutSize = 33.0f;
};

class RandomForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	RandomForGPU() = default;
	~RandomForGPU() = default;

	void ImGui();

	//--------- properties ---------------------------------------------------

	float time = 0.001f;
	int32_t enable = false;
};

class GlitchForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GlitchForGPU() = default;
	~GlitchForGPU() = default;

	void ImGui();

	//--------- properties ---------------------------------------------------

	float time = 0.0f;        // 経過秒
	float intensity = 0.0f;   // グリッチ全体の強さ
	float blockSize = 32.0f;  // 横ずれブロックの太さ
	float colorOffset = 4.0f; // RGBずれ距離

	float noiseStrength = 0.2f; // ノイズ混合率
	float lineSpeed = 1.5f;     // スキャンライン走査速度
};

class CRTDisplayForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	void ImGui();

	//--------- properties ---------------------------------------------------

	// 出力解像度
	Vector2 resolution = Vector2(Config::kWindowWidthf, Config::kWindowHeightf);
	Vector2 invResolution = 1.0f / Vector2(Config::kWindowWidthf, Config::kWindowHeightf);

	float barrel = 0.18f; // 歪曲量
	float zoom = 1.08f;   // 歪曲で欠ける分の拡大

	float cornerRadius = 64.0f; // 丸角半径
	float cornerFeather = 3.0f; // 丸角フェザー幅

	float vignette = 0.28f;  // ヴィネット強度
	float aberration = 0.0f; // 色収差の量

	float scanlineIntensity = 0.25f;              // 走査線強度
	float scanlineCount = Config::kWindowHeightf; // 走査線の本数

	float noise = 0.03f; // ノイズ強度
	float time = 0.0f;   // 秒

	float pad0[2];
};

class PlayerAfterImageForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	void ImGui();

	//--------- properties ---------------------------------------------------

	// 有効フラグ
	int32_t enable = 0;

	// ディザリング率(0.0f~1.0f)
	float rate = 1.0f;

	float pad0[2];

	// パターン
	int pattern[4][4] = {
	{  0, 32,  8, 40 },
	{ 48, 16, 56, 24 },
	{ 12, 44,  4, 36 },
	{ 60, 28, 52, 20 } };

	// ピクセルの割る数
	float divisor = 4.0f;

	float pad1[3];

	// ディザリング色
	Color ditherColor = Color::White();

	// 発光
	Vector3 emissionColor = Vector3::AnyInit(1.0f);
	float emissionIntensity = 0.0f;
};

class DefaultDistortionForGPU {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DefaultDistortionForGPU() = default;
	~DefaultDistortionForGPU() = default;

	void ImGui();

	//--------- properties ---------------------------------------------------

	// バイアス
	float bias = 1.0f;
	// 強さ
	float strength = 1.0f;

	float pad[2];

	Color color = Color::White();
	Matrix4x4 uvTransform = Matrix4x4::MakeIdentity4x4();
};