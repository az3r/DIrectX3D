#include "pch.h"
#include "WinApp.h"

std::wstring WinApp::sClassName = L"WndClass";

LRESULT WinApp::SetupWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WinApp* lpWinApp = nullptr;
	if (msg == WM_NCCREATE)
	{
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		lpWinApp = static_cast<WinApp*> (lpcs->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(lpWinApp));
	}
	else lpWinApp = reinterpret_cast<WinApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (lpWinApp) return lpWinApp->WndProc(hwnd, msg, wParam, lParam);
	else return DefWindowProc(hwnd, msg, wParam, lParam);
}
ATOM WinApp::RegisterWndClassEx()
{
	ZeroMemory(&mWC, sizeof(mWC));

	mWC.cbSize = sizeof(WNDCLASSEX);
	mWC.hInstance = g_hInstance;
	mWC.lpfnWndProc = WinApp::SetupWndProc;
	mWC.lpszClassName = sClassName.c_str();

	return RegisterClassEx(&mWC);
}

void WinApp::Move(int width, int height, int x, int y)
{
	BOOL result = MoveWindow(mHWND, x, y, width, height, TRUE);
	assert(result, Exception::TranslateErrorCode(GetLastError()));

	mWidth = width;
	mHeight = height;
	mLeft = x;
	mTop = y;
}

void WinApp::SetTitle(std::wstring title) noexcept
{
	mTitle = title;
	SetWindowText(mHWND, mTitle.c_str());
}

int WinApp::InitInstance()
{
	Exception::throw_if_false((BOOL)RegisterWndClassEx(), Exception::TranslateErrorCode(GetLastError()), __LINE__, __FILE__, "Window Registeration Failure");

	mHWND = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		sClassName.c_str(),
		mTitle.c_str(),
		WS_OVERLAPPEDWINDOW,
		mLeft, mTop, mWidth, mHeight,
		NULL,
		NULL,
		g_hInstance,
		this
	);


	Exception::throw_if_false((BOOL)mHWND, Exception::TranslateErrorCode(GetLastError()), __LINE__, __FILE__, "Window Creation Failure");

	ShowWindow(mHWND, g_nCmdShow);
	UpdateWindow(mHWND);

	return 0;
}

int WinApp::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	BOOL result;

	while (result = GetMessage(&msg, mHWND, NULL, NULL))
	{
		if (result < 0)
		{
			// window has been closed and destroyed
			return 0;
		}
		TranslateMessage(&msg);
		DispatchMessageW(&msg);

		mKeyboard.Read();
		if (mKeyboard.IsKeyPressed(VK_MENU)) MessageBox(NULL, L"A is released", NULL, NULL);

	}

	return 0;
}

int WinApp::ExitInstance()
{
	BOOL result = UnregisterClass(sClassName.c_str(), g_hInstance);
	Exception::throw_if_false(result, Exception::TranslateErrorCode(GetLastError()), __LINE__, __FILE__, "Unregisteration Failure");
	return 0;
}

LRESULT WinApp::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT painter;
		HDC hdc = BeginPaint(hwnd, &painter);

		RECT rect;
		GetClientRect(hwnd, &rect);

		FillRect(hdc, &rect, CreateSolidBrush(RGB(0x69, 0x69, 0x69)));

		EndPaint(hwnd, &painter);
		return 0;
	}
	case WM_KEYDOWN:
	{
		unsigned char key = static_cast<unsigned char>(wParam);
		KeyState state(std::bitset<3>("100"));
		// check whether this key is previously down or up
		bool prevDown = lParam & 0x40000000;
		if (!prevDown) state.SetDownAndPressed();
		mKeyboard.OnKeyEvent(KeyEventArgs(state, key));
		return 1;
	}
	case WM_SYSKEYDOWN:
	{
		unsigned char key = static_cast<unsigned char>(wParam);
		KeyState state(std::bitset<3>("100"));
		// check whether this key is previously down or up
		bool prevDown = lParam & 0x40000000;
		if (!prevDown) state.SetDownAndPressed();
		mKeyboard.OnKeyEvent(KeyEventArgs(state, key));
		return 1;
	}
	case WM_KEYUP:
		mKeyboard.OnKeyEvent(KeyEventArgs(std::bitset<3>("001"), static_cast<unsigned char>(wParam)));
		return 1;
	case WM_SYSKEYUP:
		mKeyboard.OnKeyEvent(KeyEventArgs(std::bitset<3>("001"), static_cast<unsigned char>(wParam)));
		return 1;
	case WM_KILLFOCUS:
		mKeyboard.Reset();
		return 1;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

WinApp::WinApp(std::wstring title) noexcept : WinApp()
{
	mTitle = title;
}

WinApp::WinApp(std::wstring title, int width, int height) noexcept : WinApp(title)
{
	mWidth = width;
	mHeight = height;
}

WinApp::WinApp(std::wstring title, int width, int height, int x, int y) noexcept : WinApp(title, width, height)
{
	mLeft = x;
	mTop = y;
}

WinApp::WinApp(int width, int height) noexcept : WinApp()
{
	mWidth = width;
	mHeight = height;
}

WinApp::WinApp(int width, int height, int x, int y) noexcept : WinApp(L"Desktop Application", width, height, x, y)
{
}
