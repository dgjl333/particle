#pragma once

#include <Windows.h>
#include "MathType.h"

class Window
{
public:
	Window() = delete;
	~Window() = delete;

	static void Init();

	static bool Update();

	static void Destroy();

	static int GetWidth() { return s_width; }
	static int GetPhysicalWidth() { return s_physicalWidth; }
	static int GetPhysicalHeight() { return s_physicalHeight; }
	static int GetHeight() { return s_height;}
	static float2 GetDpiScale() { return s_dpiScale; }
	static HWND GetHWND() { return s_hwnd; }

private:
	static WNDCLASSEX s_wc;
	static HWND s_hwnd;
	static MSG s_msg;
	static int s_width;
	static int s_physicalWidth;
	static int s_height;
	static int s_physicalHeight;
	static float2 s_dpiScale;

	static LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};