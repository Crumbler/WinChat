#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

void OnCreate(HWND hwnd, HINSTANCE hInstance);
void OnResize(HWND hwnd, int resizeType, int newWidth, int newHeight);
void OnPaint(HWND hwnd);
void OnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);
void OnControl(HWND hwnd, HWND hwndControl, int idNotify, int idControl);
