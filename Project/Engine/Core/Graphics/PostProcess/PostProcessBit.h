#pragma once

//============================================================================
//	PostProcessBit
//============================================================================

// 特定のポストエフェクトのビット
static const int Bit_None = 0;
static const int Bit_Bloom = 1 << 0;
static const int Bit_HorizontalBlur = 1 << 1;
static const int Bit_VerticalBlur = 1 << 2;
static const int Bit_RadialBlur = 1 << 3;
static const int Bit_GaussianFilter = 1 << 4;
static const int Bit_BoxFilter = 1 << 5;
static const int Bit_Dissolve = 1 << 6;
static const int Bit_Random = 1 << 7;
static const int Bit_Vignette = 1 << 8;
static const int Bit_Grayscale = 1 << 9;
static const int Bit_SepiaTone = 1 << 10;
static const int Bit_LuminanceBasedOutline = 1 << 11;
static const int Bit_DepthBasedOutline = 1 << 12;
static const int Bit_Lut = 1 << 13;
static const int Bit_Glitch = 1 << 14;
static const int Bit_CRTDisplay = 1 << 15;
static const int Bit_PlayerAfterImage = 1 << 16;
static const int Bit_DefaultDistortion = 1 << 17;