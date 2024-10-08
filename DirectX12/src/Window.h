#pragma once
#include <Windows.h>

class Window
{
private :
	static WNDCLASSEX m_wc;
	static HWND m_hwnd;
	static MSG m_msg;
	static int m_width;
	static int m_height;
	
	static LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	Window() = delete;
	~Window() = delete;

	static void OnStart(float size);

	static bool OnUpdate();

	static void OnDestroy();

	static int GetWidth() { return m_width; }
	static int GetHeight() { return m_height;}
	static HWND GetHWND() { return m_hwnd; }
};