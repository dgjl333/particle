#pragma once

#include "CommonData.h"

class Input
{
private:
	static bool s_mouseLeftDown;
	static bool s_mouseLeftEnterDown;
	static bool s_mouseLeftEnterUp;

public:
	static void ClearStates();

	static void UpdateMouseState(bool isButtonDown);

	static void UpdateMousePosition(float2 pos);

	static bool GetMouseButton() { return s_mouseLeftDown; }

	static bool GetMouseButtonDown() { return s_mouseLeftEnterDown; }

	static bool GetMouseButtonUp() { return s_mouseLeftEnterUp; }
};