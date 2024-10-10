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
	m_height = GetSystemMetrics(SM_CYSCREEN)* size;
	m_width = GetSystemMetrics(SM_CXSCREEN) * size;

	m_wc.cbSize = sizeof(WNDCLASSEX);
	m_wc.lpfnWndProc = (WNDPROC)Window::WindowProcedure;
	m_wc.lpszClassName = L"DX12";
	m_wc.hInstance = GetModuleHandle(nullptr);
	RegisterClassEx(&m_wc);
	RECT rect = { 0, 0, m_width, m_height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

	m_hwnd = CreateWindow(m_wc.lpszClassName, L"Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, m_wc.hInstance, nullptr);

	if (m_hwnd == NULL)
	{
		print("Window creation failed");
		exit(1);
	}

	ShowWindow(m_hwnd, SW_SHOW);
}

bool Window::Update()
{
	if (PeekMessage(&m_msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&m_msg);
		DispatchMessage(&m_msg);
	}

	if (m_msg.message == WM_QUIT)
	{
		return false;
	}
	return true;
}

void Window::Destroy()
{
	GUI::Destroy();
	DestroyWindow(m_hwnd);
	UnregisterClass(m_wc.lpszClassName, m_wc.hInstance);
}

WNDCLASSEX Window::m_wc = {};
HWND Window::m_hwnd = {};
MSG Window::m_msg = {};
int Window::m_width = 0;
int Window::m_height = 0;