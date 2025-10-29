#pragma once

//============================================================================
//	ParticleConfig
//============================================================================

static const int kMaxGPUParticles = 0xffff;
static const int kMaxCPUParticles = 0x0400;
static const int kMaxTrailParticles = 64;
static const int THREAD_EMIT_GROUP = 1024;
static const int THREAD_UPDATE_GROUP = 1024;