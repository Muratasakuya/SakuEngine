//============================================================================
//	Structures
//============================================================================

struct TrailHeader {

	uint start;
	uint vertCount;
};
struct TrailVertex {
	
	float3 worldPos;
	float2 uv;
	float4 color;
};