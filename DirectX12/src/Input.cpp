#include "Input.h"
#include "imgui/imgui.h"

std::array<bool, Input::mouseButton_count> Input::s_mouseDown = {false, false, false};
std::array<bool, Input::mouseButton_count> Input::s_mouseEnterDown = {false, false, false};
std::array<bool, Input::mouseButton_count> Input::s_mouseEnterUp = {false, false, false};
float2 Input::s_mousePos;

void Input::ClearStates()
{
	for (int i = 0; i < Input::mouseButton_count; i++)
	{
		s_mouseEnterDown[i] = false;
		s_mouseEnterUp[i] = false;
	}
}

void Input::UpdateMouseState(MouseButton button, bool isButtonDown)
{
	if (ImGui::GetIO().WantCaptureMouse) return;

	if (s_mouseDown[(int)button] == isButtonDown) return;

	s_mouseDown[(int)button] = isButtonDown;

	if (isButtonDown)
	{
		s_mouseEnterDown[(int)button] = true;
	}
	else
	{
		s_mouseEnterUp[(int)button] = true;
	}
}

void Input::UpdateMousePosition(float2 pos)
{
	s_mousePos = pos;
}
