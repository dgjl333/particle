#include "ParticleManager.h"
#include "Time.h"
#include <algorithm>

ParticleEffect ParticleManager::s_particleEffect = {
	.constantForceStrength = 0.0f,   
	.viscosity = 0.025f,
	.curlScale = 0.005f,
	.curlStrength = 75.0f,
	.mousePos = {0.0f, 0.0f}         
};

MouseEffect ParticleManager::s_mouseEffect = {};

float ParticleManager::s_targetForceStrength = 1.0f;
float ParticleManager::s_passedTime = 0.0f;
MouseButton ParticleManager::s_activeButton = MouseButton::None;

void ParticleManager::HandleInputData(float2 mousePos)
{
	if (Input::GetMouseButton(MouseButton::LEFT) || Input::GetMouseButton(MouseButton::RIGHT))
	{
		s_particleEffect.mousePos = mousePos;
		s_mouseEffect.flash = 1;
	}
	else
	{
		s_mouseEffect.flash = 0;
	}

	if (s_activeButton == MouseButton::None)
	{
		if (Input::GetMouseButtonDown(MouseButton::LEFT))
		{
			s_activeButton = MouseButton::LEFT;
		}
		else if (Input::GetMouseButtonDown(MouseButton::RIGHT))
		{
			s_activeButton = MouseButton::RIGHT;
		}
	}
	else if (s_activeButton == MouseButton::LEFT)
	{
		UpdateStrength(s_targetForceStrength);

		if (Input::GetMouseButtonUp(MouseButton::LEFT))
		{
			ClearState();
		}
	}
	else if (s_activeButton == MouseButton::RIGHT)
	{
		UpdateStrength(-2 * s_targetForceStrength);

		if (Input::GetMouseButtonUp(MouseButton::RIGHT))
		{
			ClearState();
		}
	}
}

void ParticleManager::UpdateStrength(float targetStrength)
{
	s_passedTime += Time::GetDeltaTime();
	float progress = std::clamp(s_passedTime / s_smoothTime, 0.0f, 1.0f);
	s_particleEffect.constantForceStrength = std::lerp(0, targetStrength, progress);
}

void ParticleManager::ClearState()
{
	s_particleEffect.constantForceStrength = 0;
	s_passedTime = 0;
	s_activeButton = MouseButton::None;
}