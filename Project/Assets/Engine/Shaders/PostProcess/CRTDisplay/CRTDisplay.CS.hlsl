//============================================================================
//	include
//============================================================================

#include "../../Math/Math.hlsli"
#include "../PostProcessCommon.hlsli"

//============================================================================
//	CBuffer
//============================================================================

struct CRTMaterial {
	
	float2 resolution; // 出力解像度
	float2 invResolution;
	
	float barrel; // 歪曲量
	float zoom; // 歪曲で欠ける分の拡大

	float cornerRadius; // 丸角半径
	float cornerFeather; // 丸角フェザー幅
	
	float vignette; // ヴィネット強度
	float aberration; // 色収差の量
	
	float scanlineIntensity; // 走査線強度
	float scanlineCount; // 走査線の本数
	
	float noise; // ノイズ強度
	float time; // 秒
};
ConstantBuffer<CRTMaterial> gMaterial : register(b0);

//============================================================================
//	Texture
//============================================================================

SamplerState gSampler : register(s0);

//============================================================================
//	Functions
//============================================================================

// [-1,1]空間でのバレル写像 f(x)=x*(1+k|x|^2)
float2 BarrelDistort(float2 p, float k) {
	
	float r2 = dot(p, p);
	return p * (1.0f + k * r2);
}

// 逆写像（ニュートン法）でサンプルUVを近似取得
float2 InverseBarrelUV(float2 uv, float k, float zoom) {

	float2 p = (uv * 2.0f - 1.0f) / zoom;
	float2 x = p;

	[unroll]
	for (int i = 0; i < 4; ++i) {
		
		float r2 = dot(x, x);
		float a = 1.0 + k * r2;
		float2 fx = x * a - p; // f(x)-p

		// ヤコビアン J = a*I + 2k x x^T の逆行列を解析計算
		float b = 2.0 * k;
		float xx = x.x, yy = x.y;
		float j00 = a + b * yy * yy;
		float j11 = a + b * xx * xx;
		float j01 = -b * xx * yy;
		float det = j00 * j11 - j01 * j01;
		float2 dx = float2(j11 * fx.x - j01 * fx.y, -j01 * fx.x + j00 * fx.y) / det;

		x -= dx;
	}

	return (x * zoom) * 0.5f + 0.5f;
}

// 丸角マスク
float CornerMask(float2 uv, float cornerRadiusPx, float featherPx) {
	
	float2 res = gMaterial.resolution;
	float2 p = uv * res; // pixel space
	float2 hs = res * 0.5f - cornerRadiusPx; // 半サイズ-r
	float2 center = res * 0.5f;
	float d = SdRoundRect(p - center, hs, cornerRadiusPx);
	// d<=0 が内側。外側へ向けてフェザー
	float alpha = 1.0f - SStep(0.0f, featherPx, d);
	return saturate(alpha);
}

// ヴィネット
float VignetteMask(float2 uv, float strength) {
	
	float2 c = uv - 0.5f;
	float r = length(c) * 1.41421356f; // 対角基準で~1
	float v = 1.0f - strength * r * r; // 二次
	return saturate(v);
}

// 走査線
float Scanline(float2 uv, float intensity, float count) {

	if (intensity <= 0.0f) {
		return 1.0f;
	}
	float y = uv.y * count;
	float s = 0.5f + 0.5f * cos(2.0f * PI * y);
	return lerp(1.0f, s, intensity);
}

// 色収差
float3 ApplyAberration(float2 uv, float aberr) {
	
	float2 c = uv - 0.5f;
	float2 dir = normalize(c + 1e-6f);
	float2 duv = dir * aberr; // テクセル単位
	float r = gInputTexture.SampleLevel(gSampler, uv + duv, 0).r;
	float g = gInputTexture.SampleLevel(gSampler, uv, 0).g;
	float b = gInputTexture.SampleLevel(gSampler, uv - duv, 0).b;
	return float3(r, g, b);
}

float3 AddGrain(float3 col, float2 uv, float time, float amount) {
	
	if (amount <= 0.0f) {
		
		return col;
	}
	float n = Hash12(uv * gMaterial.resolution + time * 60.0f);
	return col + (n - 0.5f) * amount;
}

//============================================================================
//	main
//============================================================================
[numthreads(THREAD_POSTPROCESS_GROUP, THREAD_POSTPROCESS_GROUP, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint width, height;
	gInputTexture.GetDimensions(width, height);

	// ピクセル位置
	uint2 pixelPos = DTid.xy;

	// 範囲外
	if (pixelPos.x >= width || pixelPos.y >= height) {
		return;
	}
	
	// フラグが立っていなければ処理しない
	if (!CheckPixelBitMask(Bit_CRTDisplay, pixelPos)) {
		
		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}

	float2 uv = (float2) DTid.xy / gMaterial.resolution;
	float2 suv = InverseBarrelUV(uv, gMaterial.barrel, gMaterial.zoom);

	// 丸角 + ヴィネット
	float mask = CornerMask(uv, gMaterial.cornerRadius, gMaterial.cornerFeather);
	mask *= VignetteMask(uv, gMaterial.vignette);

	// 範囲外はマスクで消す
	float inside = step(0.0f, suv.x) * step(0.0f, suv.y) * step(suv.x, 1.0f) * step(suv.y, 1.0f);
	mask *= inside;

	float3 color;
	if (gMaterial.aberration > 0.0f) {
		
		color = ApplyAberration(suv, gMaterial.aberration * gMaterial.invResolution.x);
	} else {
		
		color = gInputTexture.SampleLevel(gSampler, suv, 0).rgb;
	}

	// 走査線
	color *= Scanline(uv, gMaterial.scanlineIntensity, max(gMaterial.scanlineCount, 1.0f));

	// ノイズ
	color = AddGrain(color, uv, gMaterial.time, gMaterial.noise);

	// マスク適用
	color *= mask;

	gOutputTexture[DTid.xy] = float4(color, 1.0f);
}