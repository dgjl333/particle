#pragma once
#include <Windows.h>

class Window
{
public:
	Window() = delete;
	~Window() = delete;

	static void Init();

	static bool Update();

	static void Destroy();

	static int GetWidth() { return s_width; }
	static int GetHeight() { return s_height;}
	static HWND GetHWND() { return s_hwnd; }

private:
	static WNDCLASSEX s_wc;
	static HWND s_hwnd;
	static MSG s_msg;
	static int s_width;
	static int s_height;

	static LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};