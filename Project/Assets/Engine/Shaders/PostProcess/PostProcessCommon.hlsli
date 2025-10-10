//============================================================================
//	include
//============================================================================

#include "../../../../Engine/Core/Graphics/PostProcess/PostProcessConfig.h"
#include "../../../../Engine/Core/Graphics/PostProcess/PostProcessBit.h"

//============================================================================
//	Texture
//============================================================================

Texture2D<float4> gInputTexture : register(t0);
Texture2D<uint4> gMaskTexture : register(t1);
RWTexture2D<float4> gOutputTexture : register(u0);

//============================================================================
//	fFunctions
//============================================================================

bool CheckPixelBitMask(int bit, int mask) {
	
	return (mask & bit) != 0u;
}