//============================================================================
//	Structures
//============================================================================

static const uint TRAIL_MAX_VERTS = 256;
static const uint TRAIL_MAX_PRIMS = TRAIL_MAX_VERTS - 2;

struct TrailHeader {

	uint start;
	uint vertCount;
};
struct TrailVertex {
	
	float3 worldPos;
	float2 uv;
	float4 color;
};