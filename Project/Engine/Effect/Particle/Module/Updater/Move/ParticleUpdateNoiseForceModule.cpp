#include "ParticleUpdateNoiseForceModule.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	ParticleUpdateNoiseForceModule classMethods
//============================================================================

// Perlin補間
static inline float Fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
static inline uint32_t Hash3d(int x, int y, int z, uint32_t s) {

	uint32_t h = (uint32_t)x * 374761393u + (uint32_t)y * 668265263u + (uint32_t)z * 362437u + s * 521417u;
	h ^= h >> 13; h *= 1274126177u; h ^= h >> 16; return h;
}
static inline float Rnd01(uint32_t h) { return (h & 0x00FFFFFFu) / float(0x01000000); }

void ParticleUpdateNoiseForceModule::Init() {

	// 初期化値
	octaves_ = 3;
	frequency_ = 0.5f;
	timeScale_ = 0.3f;
	strength_ = 1.0f;
	damping_ = 0.0f;
	seed_ = 1337;
	anchorToSpawn_ = true;
	offsetAmp_ = Vector3::AnyInit(0.2f);
}

void ParticleUpdateNoiseForceModule::Execute(
	CPUParticle::ParticleData& particle, float deltaTime) {

	// 基準の位置を取得
	Vector3 pos = particle.transform.translation;
	Vector3 sample = pos * frequency_ + Vector3(0.0f, timeScale_ * particle.currentTime, 0.0f);

	if (mode_ == NoiseMode::Offset) {

		// 発生位置からのオフセット揺らぎ
		Vector3 o(FBm(sample, seed_ + 11u), FBm(sample, seed_ + 23u), FBm(sample, seed_ + 31u));
		particle.transform.translation = (anchorToSpawn_ ? particle.spawnTranlation : pos)
			+ Vector3(o.x * offsetAmp_.x, o.y * offsetAmp_.y, o.z * offsetAmp_.z);
		return;
	}

	// 勾配またはカールを力として速度に加算
	Vector3 force = (mode_ == NoiseMode::Curl) ? CurlNoise(sample) : GradNoise(sample, seed_);
	particle.velocity += force * strength_;

	// 簡易ダンピング
	if (damping_ > 0.0f) {

		particle.velocity *= (std::max)(0.0f, 1.0f - damping_ * deltaTime);
	}
}

void ParticleUpdateNoiseForceModule::ImGui() {

	EnumAdapter<NoiseMode>::Combo("mode", &mode_);
	ImGui::DragInt("octaves", &octaves_, 1, 1, 8);
	ImGui::DragFloat("frequency", &frequency_, 0.01f, 0.01f, 10.0f);
	ImGui::DragFloat("timeScale", &timeScale_, 0.01f, 0.0f, 5.0f);
	if (mode_ != NoiseMode::Offset) {

		ImGui::DragFloat("strength", &strength_, 0.01f, 0.0f, 20.0f);
		ImGui::DragFloat("damping", &damping_, 0.001f, 0.0f, 1.0f);
	} else {

		ImGui::DragFloat3("offsetAmp", &offsetAmp_.x, 0.01f);
		ImGui::Checkbox("anchorToSpawn", &anchorToSpawn_);
	}
	ImGui::InputScalar("seed", ImGuiDataType_U32, &seed_);
}

Json ParticleUpdateNoiseForceModule::ToJson() {

	Json data;

	data["octaves_"] = octaves_;
	data["frequency_"] = frequency_;
	data["timeScale_"] = timeScale_;
	data["strength_"] = strength_;
	data["damping_"] = damping_;
	data["seed_"] = seed_;
	data["anchorToSpawn_"] = anchorToSpawn_;
	data["offsetAmp_"] = offsetAmp_.ToJson();

	return data;
}

void ParticleUpdateNoiseForceModule::FromJson(const Json& data) {

	octaves_ = data.value("octaves_", 3);
	frequency_ = data.value("frequency_", 0.5f);
	timeScale_ = data.value("timeScale_", 0.3f);
	strength_ = data.value("strength_", 1.0f);
	damping_ = data.value("damping_", 0.0f);
	seed_ = data.value("seed_", 1337);
	anchorToSpawn_ = data.value("anchorToSpawn_", true);
	offsetAmp_ = Vector3::FromJson(data.value("offsetAmp_", Json()));
}

float ParticleUpdateNoiseForceModule::Noise3(const Vector3& p, uint32_t s) const {

	const int X = (int)floorf(p.x), Y = (int)floorf(p.y), Z = (int)floorf(p.z);
	const float fx = p.x - X, fy = p.y - Y, fz = p.z - Z;
	const float ux = Fade(fx), uy = Fade(fy), uz = Fade(fz);
	auto L = [&](int dx, int dy, int dz) { return Rnd01(Hash3d(X + dx, Y + dy, Z + dz, s)); };
	// 8隅をtrilinear
	float x00 = std::lerp(L(0, 0, 0), L(1, 0, 0), ux);
	float x10 = std::lerp(L(0, 1, 0), L(1, 1, 0), ux);
	float x01 = std::lerp(L(0, 0, 1), L(1, 0, 1), ux);
	float x11 = std::lerp(L(0, 1, 1), L(1, 1, 1), ux);
	float y0 = std::lerp(x00, x10, uy);
	float y1 = std::lerp(x01, x11, uy);

	// [-1,1]
	return std::lerp(y0, y1, uz) * 2.0f - 1.0f;
}

float ParticleUpdateNoiseForceModule::FBm(const Vector3& p, uint32_t s) const {

	float a = 1.0f, f = 1.0f, sum = 0.0f, norm = 0.0f;
	for (int i = 0; i < octaves_; i++) {

		sum += a * Noise3(p * f, s + i * 101u);
		norm += a; a *= 0.5f; f *= 2.0f;
	}
	return sum / (std::max)(1e-6f, norm);
}

Vector3 ParticleUpdateNoiseForceModule::GradNoise(const Vector3& p, uint32_t s) const {

	const float eps = 0.01f;
	const float nx1 = FBm(Vector3(p.x + eps, p.y, p.z), s), nx0 = FBm(Vector3(p.x - eps, p.y, p.z), s);
	const float ny1 = FBm(Vector3(p.x, p.y + eps, p.z), s), ny0 = FBm(Vector3(p.x, p.y - eps, p.z), s);
	const float nz1 = FBm(Vector3(p.x, p.y, p.z + eps), s), nz0 = FBm(Vector3(p.x, p.y, p.z - eps), s);
	return Vector3((nx1 - nx0) / (2 * eps), (ny1 - ny0) / (2 * eps), (nz1 - nz0) / (2 * eps));
}

Vector3 ParticleUpdateNoiseForceModule::CurlNoise(const Vector3& p) const {

	const float eps = 0.01f;
	auto n1 = [&](const Vector3& q) { return FBm(q, seed_ + 17u); };
	auto n2 = [&](const Vector3& q) { return FBm(q, seed_ + 37u); };
	auto n3 = [&](const Vector3& q) { return FBm(q, seed_ + 59u); };
	// 中心差分で curl(F) = (∂Fz/∂y-∂Fy/∂z, ∂Fx/∂z-∂Fz/∂x, ∂Fy/∂x-∂Fx/∂y)
	float dFz_dy = (n3(p + Vector3(0, eps, 0)) - n3(p - Vector3(0, eps, 0))) / (2 * eps);
	float dFy_dz = (n2(p + Vector3(0, 0, eps)) - n2(p - Vector3(0, 0, eps))) / (2 * eps);
	float dFx_dz = (n1(p + Vector3(0, 0, eps)) - n1(p - Vector3(0, 0, eps))) / (2 * eps);
	float dFz_dx = (n3(p + Vector3(eps, 0, 0)) - n3(p - Vector3(eps, 0, 0))) / (2 * eps);
	float dFy_dx = (n2(p + Vector3(eps, 0, 0)) - n2(p - Vector3(eps, 0, 0))) / (2 * eps);
	float dFx_dy = (n1(p + Vector3(0, eps, 0)) - n1(p - Vector3(0, eps, 0))) / (2 * eps);
	return Vector3(dFz_dy - dFy_dz, dFx_dz - dFz_dx, dFy_dx - dFx_dy);
}