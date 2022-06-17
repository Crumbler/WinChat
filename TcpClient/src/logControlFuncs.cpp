
#include "logControl.hpp"
#include "logControlFuncs.hpp"

#include <gdiplus.h>

using namespace Gdiplus;

LogControlExtra* GetLogControl(HWND hwnd)
{
    return (LogControlExtra*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}

void OnLogCreate(HWND hwnd)
{
    auto *logCtrl = new LogControlExtra();

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)logCtrl);
}

void OnLogClose(HWND hwnd)
{
    auto *logCtrl = GetLogControl(hwnd);

    delete logCtrl;
}

void OnLogPaint(HWND hwnd)
{
    auto *logCtrl = GetLogControl(hwnd);

    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    SolidBrush brush(Color::Beige);

    graphics.FillRectangle(&brush, 0, 0, logCtrl->width, logCtrl->height);

    EndPaint(hwnd, &ps);
}

void OnLogResize(HWND hwnd, int newWidth, int newHeight)
{
    auto *logCtrl = GetLogControl(hwnd);

    logCtrl->width = newWidth;
    logCtrl->height = newHeight;
}

void OnLogAdd(HWND hwnd)
{

}

void OnLogClear(HWND hwnd)
{

}
