#include <iostream>
#include <string>

#include <Windows.h>
#include <windowsx.h>

#include <winrt/windows.media.control.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.foundation.collections.h>

class WindowMgr {
private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		if (msg == WM_PAINT) {
			HBRUSH hbr = CreateSolidBrush(RGB(255, 0, 0));
			HBRUSH black = CreateSolidBrush(RGB(0, 0, 0));

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			FillRect(hdc, &ps.rcPaint, hbr);

			HGDIOBJ oldObj = SelectObject(hdc, black);

			std::string serverAddress = "Server Address: ";
			std::string clientAddress = "Client Address: ";


			// hacky hack
			RECT drawRectangle = ps.rcPaint;
			DrawTextA(hdc, "pigga", 5, &ps.rcPaint, DT_CENTER);
			DrawTextA(hdc, serverAddress.data(), serverAddress.length(), &drawRectangle, DT_CENTER);
			drawRectangle.top += 20;
			DrawTextA(hdc, clientAddress.data(), clientAddress.length(), &drawRectangle, DT_CENTER);

			SelectObject(hdc, oldObj);

			EndPaint(hwnd, &ps);

			DeleteBrush(hbr);
			DeleteBrush(black);
		}
		else if (msg == WM_CLOSE) {
			DestroyWindow(hwnd);
		}
		else if (msg == WM_DESTROY) {
			PostQuitMessage(0);
			return 0;
		}

		return DefWindowProc(hwnd, msg, wp, lp);
	}

public:
	LONG width, height;
	std::string title;

	void InitWindow() {
		HINSTANCE hinst = GetModuleHandle(0);

		WNDCLASSEXA wcex = {};
		wcex.cbSize = sizeof wcex;
		wcex.lpfnWndProc = WndProc;
		wcex.lpszClassName = "Main class";
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.hInstance = hinst;

		RegisterClassExA(&wcex);

		RECT clientRect = { 0, 0, width, height };
		AdjustWindowRectEx(&clientRect, WS_OVERLAPPEDWINDOW, 0, WS_EX_OVERLAPPEDWINDOW);
		HWND hwnd = CreateWindowExA(
			WS_EX_OVERLAPPEDWINDOW, "Main class", title.data(), WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top,
			0, 0, hinst, 0
		);

		ShowWindow(hwnd, SW_SHOW);
	}

	WindowMgr(std::string title, LONG width, LONG height) {
		this->title = title;
		this->width = width;
		this->height = height;
	}
};