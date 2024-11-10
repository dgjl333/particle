#ifndef PARTICLE_DATA
#define PARTICLE_DATA

#define PARTICLE_COUNT 500000
#define PARTICLE_NUMTHREADS 32

static float s_size = 1.25;

static float4 s_orange = float4(1, 0.4291667, 0, 1);
static float4 s_blue = float4(0, 0.1764705, 0.7490196, 1);
static float4 s_purple = float4(0.6823529, 0.2156862, 1, 1);
static float4 s_red = float4(0.749019, 0.098039, 0.011764, 1);

static float s_noiseStrength = 75;
static float s_noiseScale = 0.005;
static float s_viscosity = 0.025;
static float s_damping = 0.2;

#endif