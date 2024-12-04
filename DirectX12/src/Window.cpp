#include "Window.h"
#include "Debug.h"
#include "GUI.h"
#include "Input.h"
#include "imgui/imgui_impl_win32.h"
#include <windowsx.h>
#include <format>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Window::WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lParam)) return true;

	switch (msg)
	{
	case WM_MOUSEMOVE:
	{
		float x = GET_X_LPARAM(lParam);
		float y = s_height - GET_Y_LPARAM(lParam);
		Input::UpdateMousePosition({ x,y });
		return 0;
	}
	case WM_LBUTTONDOWN:
		Input::UpdateMouseState(MouseButton::LEFT, true);
		return 0;
	case WM_LBUTTONUP:
		Input::UpdateMouseState(MouseButton::LEFT, false);
		return 0;
	case WM_RBUTTONDOWN:
		Input::UpdateMouseState(MouseButton::RIGHT, true);
		return 0;
	case WM_RBUTTONUP:
		Input::UpdateMouseState(MouseButton::RIGHT, false);
		return 0;
	case WM_MBUTTONDOWN:
		Input::UpdateMouseState(MouseButton::MIDDLE, true);
		return 0;
	case WM_MBUTTONUP:
		Input::UpdateMouseState(MouseButton::MIDDLE, false);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wparam, lParam);
	}
}

void Window::Init()
{
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	HMONITOR hMonitor = MonitorFromPoint(cursorPos, MONITOR_DEFAULTTONEAREST);

	MONITORINFOEXA monitorInfo = {};
	monitorInfo.cbSize = sizeof(MONITORINFOEXA);

	GetMonitorInfoA(hMonitor, &monitorInfo);
	
	s_width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
	s_height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

	s_wc.cbSize = sizeof(WNDCLASSEX);
	s_wc.lpfnWndProc = (WNDPROC)Window::WindowProcedure;
	s_wc.lpszClassName = L"DX12";
	s_wc.hInstance = GetModuleHandle(nullptr);
	RegisterClassEx(&s_wc);
	RECT rect = { 0, 0, s_width, s_height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	
	s_hwnd = CreateWindow(s_wc.lpszClassName, L"Particle", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, s_width, s_height, nullptr, nullptr, s_wc.hInstance, nullptr);

	if (s_hwnd == NULL)
	{
		print("Window creation failed");
		exit(1);
	}

	LONG style = GetWindowLong(s_hwnd, GWL_STYLE);
	style &= ~WS_THICKFRAME;      // Remove the resize
	SetWindowLong(s_hwnd, GWL_STYLE, style);
	
	ShowWindow(s_hwnd, SW_SHOWMAXIMIZED);
	
	HMONITOR monitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFOEX info = { sizeof(MONITORINFOEX) };
	GetMonitorInfo(monitor, &info);
	DEVMODE devmode = {};
	devmode.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(info.szDevice, ENUM_CURRENT_SETTINGS, &devmode);

	s_physicalWidth = devmode.dmPelsWidth;
	s_physicalHeight = devmode.dmPelsHeight;

	RECT clientRect;
	GetClientRect(Window::GetHWND(), &clientRect);
	
	s_dpiScale.x = (clientRect.right - clientRect.left) / (float)devmode.dmPelsWidth;
	s_dpiScale.y = (clientRect.bottom - clientRect.top) / (float)devmode.dmPelsHeight;
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
int Window::s_physicalWidth = 0;
int Window::s_height = 0;
int Window::s_physicalHeight = 0;
float2 Window::s_dpiScale;