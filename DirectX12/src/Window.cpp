#include "Window.h"
#include "Debug.h"
#include "GUI.h"
#include "imgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Window::WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) return true;
		
	if (msg == WM_DESTROY)
	{	
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void Window::Init(float size)
{
	//m_height = GetSystemMetrics(SM_CYSCREEN)* size;
	//m_width = GetSystemMetrics(SM_CXSCREEN) * size;
	s_height = 1080 * size;         //set a fixed size for now
	s_width = 1920 * size;

	s_wc.cbSize = sizeof(WNDCLASSEX);
	s_wc.lpfnWndProc = (WNDPROC)Window::WindowProcedure;
	s_wc.lpszClassName = L"DX12";
	s_wc.hInstance = GetModuleHandle(nullptr);
	RegisterClassEx(&s_wc);
	RECT rect = { 0, 0, s_width, s_height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

	s_hwnd = CreateWindow(s_wc.lpszClassName, L"Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, s_wc.hInstance, nullptr);

	if (s_hwnd == NULL)
	{
		print("Window creation failed");
		exit(1);
	}

	ShowWindow(s_hwnd, SW_SHOW);
}

bool Window::Update()
{
	if (PeekMessage(&s_msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&s_msg);
		DispatchMessage(&s_msg);
	}

	if (s_msg.message == WM_QUIT)
	{
		return false;
	}
	return true;
}

void Window::Destroy()
{
	GUI::Destroy();
	DestroyWindow(s_hwnd);
	UnregisterClass(s_wc.lpszClassName, s_wc.hInstance);
}

WNDCLASSEX Window::s_wc = {};
HWND Window::s_hwnd = {};
MSG Window::s_msg = {};
int Window::s_width = 0;
int Window::s_height = 0;