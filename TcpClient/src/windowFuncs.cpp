#include "windowFuncs.hpp"

#include <cstdio>

#include <gdiplus.h>

using namespace Gdiplus;

constexpr int panelWidth = 200;

int clientWidth, clientHeight;

void OnCreate(HWND hwnd, HINSTANCE hInstance)
{

}

void OnResize(HWND hwnd, int resizeType, int newWidth, int newHeight)
{
    clientWidth = newWidth;
    clientHeight = newHeight;
}

void OnExitSizeMove(HWND hwnd)
{

}

void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    graphics.Clear(Color::White);

    SolidBrush brush(Color(230, 230, 230));

    graphics.FillRectangle(&brush, 0, 0, panelWidth, clientHeight);

    EndPaint(hwnd, &ps);
}

void OnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    // Not menu and not accelerator
    if (lParam != 0 && lParam != 1)
    {
        OnControl(hwnd, (HWND)lParam, HIWORD(wParam), LOWORD(wParam));
    }
}

void OnControl(HWND hwnd, HWND hwndControl, int idNotify, int idControl)
{

}
