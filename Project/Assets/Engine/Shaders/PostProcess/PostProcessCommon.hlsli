//============================================================================
//	include
//============================================================================

#include "../../../../Engine/Core/Graphics/PostProcess/PostProcessConfig.h"
#include "../../../../Engine/Core/Graphics/PostProcess/PostProcessBit.h"

//============================================================================
//	Texture
//============================================================================

Texture2D<float4> gInputTexture : register(t0);
Texture2D<uint> gMaskTexture : register(t1);
RWTexture2D<float4> gOutputTexture : register(u0);

//============================================================================
//	fFunctions
//============================================================================

// 指定ピクセル座標のビットマスク値をチェックする
bool CheckPixelBitMask(uint bit, uint2 pixelPos) {
	
	// 0でないならビットが立っているためポストエフェクト処理を行う
	return (gMaskTexture[pixelPos].r & bit) != 0;
}