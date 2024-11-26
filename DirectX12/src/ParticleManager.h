#pragma once
#include "MathType.h"
#include "Input.h"

struct Particle
{
	float2 position;
	float2 velocity;
};

struct ParticleEffect
{
	float constantForceStrength;
	float viscosity;
	float curlScale;
	float curlStrength;

	float2 mousePos;
};

struct MouseEffect
{
	float flash;
};

class ParticleManager
{
public:
	static ParticleEffect s_particleEffect;
	static MouseEffect s_mouseEffect;

	static void HandleInputData(float2 mousePos);

private:
	inline static const float s_smoothTime = 1.25;

	static float s_targetForceStrength;
	static float s_passedTime;
	static MouseButton s_activeButton;

	static void UpdateStrength(float targetStrength);
	static void ClearState();
};