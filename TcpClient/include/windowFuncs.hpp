#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

constexpr int WM_DISCONNECTED = WM_USER + 1,
    WM_ADDTOLOG = WM_DISCONNECTED + 1;

void OnCreate(HWND hwnd, HINSTANCE hInstance);
void OnResize(HWND hwnd, int resizeType, int newWidth, int newHeight);
void OnPaint(HWND hwnd);
void OnDisconnect();
void OnAddToLog(char *s);
void OnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);
void OnControl(HWND hwnd, HWND hwndControl, int idNotify, int idControl);
