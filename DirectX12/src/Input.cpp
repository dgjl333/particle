#include "Input.h"
#include "GUI.h"

bool Input::s_mouseLeftDown = false;
bool Input::s_mouseLeftEnterDown = false;
bool Input::s_mouseLeftEnterUp = false;

void Input::ClearStates()
{
	s_mouseLeftEnterDown = false;
	s_mouseLeftEnterUp = false;
}

void Input::UpdateMouseState(bool isButtonDown)
{
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

}
