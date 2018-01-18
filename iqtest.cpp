#ifndef UNICODE
	#undef _UNICODE
	#pragma message "WARNING:: Unicode NOT enabled"
#else
	#ifndef _UNICODE
		#define _UNICODE
	#endif
#endif

#define OEMRESOURCE  // for OCR_*
#include <windows.h>

namespace IQTest
{

const UINT MsgStartTest = WM_USER + 15;

LRESULT CALLBACK keyfilter(int code, WPARAM wparam, LPARAM lparam)
{
	if (code == HC_ACTION) {
		const auto p = (KBDLLHOOKSTRUCT*)lparam;
		const bool alt = p->flags & LLKHF_ALTDOWN;
		const bool ctrl = GetAsyncKeyState(VK_CONTROL) >> ((sizeof(SHORT) * 8) - 1);
		if (p->vkCode == VK_LWIN || p->vkCode == VK_RWIN)
			return 1;
		if (p->vkCode == VK_TAB && alt)
			return 1;
		if (p->vkCode == VK_ESCAPE && (alt || ctrl))
			return 1;
	}
	return CallNextHookEx(NULL, code, wparam, lparam);
}

class KeyBlocker
{
	HHOOK _hook = NULL;

public:
	explicit KeyBlocker()
		: _hook(SetWindowsHookEx(WH_KEYBOARD_LL, keyfilter, GetModuleHandle(NULL), 0))
	{ }
	virtual ~KeyBlocker()
	{
		if (_hook != NULL)
			UnhookWindowsHookEx(_hook);
	}
};

class MouseLocker
{
	class Hider
	{
	public:
		explicit Hider() { while (ShowCursor(FALSE) >= 0); }
		virtual ~Hider() { while (ShowCursor(TRUE) < 0); }
	} _hider;

	class Restorer
	{
		POINT _pos;

	public:
		explicit Restorer() { GetCursorPos(&_pos); }
		virtual ~Restorer() { SetCursorPos(_pos.x, _pos.y); }
	} _restorer;

	class Clipper
	{
	public:
		virtual ~Clipper() { ClipCursor(NULL); }
		void clip()
		{
			RECT r { 0, 0, 1, 1 };
			ClipCursor(&r);
		}
	} _clipper;

	class Capturer
	{
	public:
		virtual ~Capturer() { ReleaseCapture(); }
		void capture(HWND hwnd) { SetCapture(hwnd); }
	} _capturer;

public:
	explicit MouseLocker(HWND hwnd)
	{
		mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, 0, 0, 0, 0);
		_clipper.clip();
		_capturer.capture(hwnd);
	}
};

static void iq_test(HWND hwnd)
{	
	KeyBlocker blocker;
	MouseLocker locker(hwnd);

	MessageBox(hwnd, L"Do you know how to use the keyboard?", L"Hardware IQ Test",
		MB_OK | MB_ICONEXCLAMATION);
}

LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case MsgStartTest:
		iq_test(hwnd);
		DestroyWindow(hwnd);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

class CaptureWindow
{
	static const wchar_t* ClassName;

	static class WindowClass
	{
	public:
		explicit WindowClass()
		{
			WNDCLASSEX wc { 0 };
			wc.cbSize = sizeof(WNDCLASSEX);
			wc.lpfnWndProc = wndproc;
			wc.hInstance = GetModuleHandle(NULL);
			wc.lpszClassName = ClassName;
			RegisterClassEx(&wc);
		}
	} s_wc;

	HWND _hwnd;

public:
	explicit CaptureWindow()
		: _hwnd(CreateWindowEx(0, ClassName, L"", WS_POPUP | WS_VISIBLE,
					0, 0, 1, 1, NULL, NULL, GetModuleHandle(NULL), NULL))
	{ }
	virtual ~CaptureWindow()
	{
		if (_hwnd != NULL)
			DestroyWindow(_hwnd);
	}

	operator HWND() const { return _hwnd; }
};

/*static*/ const wchar_t* CaptureWindow::ClassName = L"wc_011518_hw";
/*static*/ CaptureWindow::WindowClass CaptureWindow::s_wc;

static int message_loop()
{
	MSG msg;
	BOOL ret;
	while ((ret = GetMessage(&msg, NULL, 0, 0) != 0) != 0) {
		if (ret == -1)
			return -1;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

} // namespace IQTest

int wmain(int argc, wchar_t* argv[])
{
	using namespace IQTest;
	CaptureWindow wnd;
	PostMessage(wnd, MsgStartTest, 0, 0);
	return message_loop();
}
