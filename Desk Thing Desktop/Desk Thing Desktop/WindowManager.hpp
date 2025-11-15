#pragma once

#include <iostream>
#include <string>

#include <Windows.h>
#include <windowsx.h>

#include <winrt/windows.media.control.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.foundation.collections.h>

class WindowMgr {
private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

public:
	LONG width, height;
	std::string title;

	void InitWindow();
	WindowMgr(std::string title, LONG width, LONG height);
};