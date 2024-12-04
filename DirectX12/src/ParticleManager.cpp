#include "ParticleManager.h"
#include "Time.h"
#include <algorithm>
#include "imgui/imgui.h"
#include "Window.h"

ParticleEffect ParticleManager::s_particleEffect = {
	.drag = 2.3f,
	.curlScale = 5.0f,
	.curlStrength = 7.5f,
	.turbulence = 5.0f,

	.mousePos = {0.0f, 0.0f},       
	.currentMouseForceStrength = 0.0f,   
};

MouseEffect ParticleManager::s_mouseEffect = {};

float ParticleManager::s_mouseTargetForceStrength = 1.0f;
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
		UpdateStrength(s_mouseTargetForceStrength);

		if (Input::GetMouseButtonUp(MouseButton::LEFT))
		{
			ClearState();
		}
	}
	else if (s_activeButton == MouseButton::RIGHT)
	{
		UpdateStrength(-2 * s_mouseTargetForceStrength);

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
	s_particleEffect.currentMouseForceStrength = std::lerp(0, targetStrength, progress);
}

void ParticleManager::ClearState()
{
	s_particleEffect.currentMouseForceStrength = 0;
	s_passedTime = 0;
	s_activeButton = MouseButton::None;
}

void ParticleManager::DrawInspector()
{
	ImGui::SetNextWindowPos({ (float)Window::GetWidth(),0 }, ImGuiCond_Once, { 1,0 });
	ImGui::SetNextWindowSize({ (float)Window::GetWidth() * 0.15f, (float)Window::GetHeight() }, ImGuiCond_FirstUseEver);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::Begin("Property", nullptr, ImGuiWindowFlags_NoMove);

	ImGui::SliderFloat("Noise Scale", &s_particleEffect.curlScale, 1.0f, 10.0f);
	ImGui::SliderFloat("Noise Strength", &s_particleEffect.curlStrength, 0.0f, 20.0f);
	ImGui::SliderFloat("Drag", &s_particleEffect.drag, 0.0f, 5.0f);
	ImGui::SliderFloat("Turbulence", &s_particleEffect.turbulence, 0.0f, 10.0f);
	ImGui::Separator();
	ImGui::SliderFloat("Mouse Strength", &s_mouseTargetForceStrength, 0.0f, 5.0f);

	ImGui::End();
	ImGui::PopStyleVar();
}