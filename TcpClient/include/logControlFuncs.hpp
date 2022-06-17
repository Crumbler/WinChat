#pragma once

#include <windows.h>

void OnLogCreate(HWND hwnd);
void OnLogClose(HWND hwnd);
void OnLogPaint(HWND hwnd);
void OnLogResize(HWND hwnd, int newWidth, int newHeight);
void OnLogAdd(HWND hwnd);
void OnLogClear(HWND hwnd);
