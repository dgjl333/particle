#include "Input.h"
#include "GUI.h"
#include "imgui/imgui.h"

bool Input::s_mouseLeftDown = false;
bool Input::s_mouseLeftEnterDown = false;
bool Input::s_mouseLeftEnterUp = false;
float2 Input::s_mousePos;

void Input::ClearStates()
{
	s_mouseLeftEnterDown = false;
	s_mouseLeftEnterUp = false;
}

void Input::UpdateMouseState(bool isButtonDown)
{
	if (ImGui::GetIO().WantCaptureMouse) return;

	if (s_mouseLeftDown == isButtonDown)
	{
		return;
	}

	s_mouseLeftDown = isButtonDown;

	if (isButtonDown)
	{
		s_mouseLeftEnterDown = true;
	}
	else
	{
		s_mouseLeftEnterUp = true;
	}
}

void Input::UpdateMousePosition(float2 pos)
{
	s_mousePos = pos;
}
