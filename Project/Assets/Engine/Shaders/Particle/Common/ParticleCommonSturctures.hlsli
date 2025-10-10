//============================================================================
//	Structures
//============================================================================

struct Transform {
	
	float3 translation;
	float3 scale;
	float4x4 rotationMatrix;
	float4x4 parentMatrix;
	
	uint billboardMode;
	uint aliveParent;
};

struct Material {
	
	float4 color;
	uint postProcessMask;
};

struct Particle {
	
	float lifeTime;
	float currentTime;

	float3 velocity;
};

struct PerFrame {
	
	float time;
	float deltaTime;
};

struct PerView {
	
	float3 cameraPos;
	
	float4x4 viewProjection;
	float4x4 billboardMatrix;
};

//============================================================================
//	Functions
//============================================================================

float4x4 MakeWorldMatrix(Transform transform, float4x4 billboardMatrix, float3 cameraPos) {
	
	// scale
	float4x4 scaleMatrix = float4x4(
		transform.scale.x, 0.0f, 0.0f, 0.0f,
		0.0f, transform.scale.y, 0.0f, 0.0f,
		0.0f, 0.0f, transform.scale.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	
	// fullBillboard
	if (transform.billboardMode == 0) {
		
		return mul(scaleMatrix, billboardMatrix);
	}
	
	// yAxisBillboard
	if (transform.billboardMode == 1) {
		
		// オブジェクト→カメラの水平成分
		float3 toCam = cameraPos - transform.translation;
		float3 f = float3(toCam.x, 0.0f, toCam.z);
		float eps = 1e-5f;
		if (dot(f, f) < eps) {

			f = float3(0.0f, 0.0f, 1.0f);
		}
		f = normalize(f);

		// 右・上
		float3 up = float3(0.0f, 1.0f, 0.0f);
		float3 r = normalize(cross(up, f));
		// 再直交化
		float3 u = cross(f, r);

		// Y軸ビルボード行列(回転＋平行移動)
		float4x4 Ry;
		Ry[0] = float4(r, 0.0f); // X行: 右
		Ry[1] = float4(u, 0.0f); // Y行: 上
		Ry[2] = float4(f, 0.0f); // Z行: 前
		// 平行移動
		Ry[3] = float4(transform.translation, 1.0f);

		float4x4 world = mul(scaleMatrix, mul(transform.rotationMatrix, Ry));
		return world;
	}
	
	return mul(scaleMatrix, transform.rotationMatrix);
}

float4x4 SetParent(Transform transform, float4x4 worldMatrix) {
	
	// 親の設定がいらない場合はそのまま返す
	if (transform.aliveParent == 0) {
		
		return worldMatrix;
	}
	
	 // 親の平行移動成分
	float3 parentPos = transform.parentMatrix[3].xyz;
	worldMatrix[3].xyz += parentPos;

	return worldMatrix;
}