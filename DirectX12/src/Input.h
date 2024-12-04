#pragma once

#include <array>
#include "MathType.h"

enum class MouseButton
{
	LEFT, RIGHT, MIDDLE, None
};

class Input
{
public:
	static constexpr int mouseButton_count = 3;

	static void ClearStates();

	static void UpdateMouseState(MouseButton button, bool isButtonDown);

	static void UpdateMousePosition(float2 pos);

	static bool GetMouseButton(MouseButton button) { return s_mouseDown[(int)button]; }

	static bool GetMouseButtonDown(MouseButton button) { return s_mouseEnterDown[(int)button]; }

	static bool GetMouseButtonUp(MouseButton button) { return s_mouseEnterUp[(int)button]; }

	static float2 GetMouseWorldPos() { return s_mouseWorldPos; }

private:
	static std::array<bool, mouseButton_count> s_mouseDown;
	static std::array<bool, mouseButton_count> s_mouseEnterDown;
	static std::array<bool, mouseButton_count> s_mouseEnterUp;
	static float2 s_mouseWorldPos;
};