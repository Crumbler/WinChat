
#include "logControl.hpp"
#include "logControlFuncs.hpp"


LRESULT CALLBACK LogWndProc(HWND, UINT, WPARAM, LPARAM);

void RegisterLogControl(HINSTANCE hInstance)
{
    WNDCLASSEXW wincl;

    wincl.hInstance = hInstance;
    wincl.lpszClassName = szLogControl;
    wincl.lpfnWndProc = LogWndProc;
    wincl.style = 0;
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wincl.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wincl.lpszMenuName = nullptr;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

    RegisterClassExW(&wincl);
}

LRESULT CALLBACK LogWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        OnLogCreate(hwnd);
        break;

    case WM_CLOSE:
        OnLogClose(hwnd);
        break;

    case WM_SIZE:
        OnLogResize(hwnd, LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_PAINT:
        OnLogPaint(hwnd);
        break;

    case WM_VSCROLL:
        OnLogVScroll(hwnd, wParam);
        break;

    case WM_LOGADD:
        OnLogAdd(hwnd, (const char*)wParam);
        break;

    case WM_LOGCLEAR:
        OnLogClear(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}
