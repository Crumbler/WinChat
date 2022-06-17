#pragma once

#include <windows.h>

void OnLogCreate(HWND hwnd);
void OnLogClose(HWND hwnd);
void OnLogPaint(HWND hwnd);
void OnLogResize(HWND hwnd, int newWidth, int newHeight);
void OnLogVScroll(HWND hwnd, WPARAM wParam);
void OnLogAdd(HWND hwnd, const char *m);
void OnLogClear(HWND hwnd);
