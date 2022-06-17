
#include <cstdio>
#include "windowFuncs.hpp"
#include <windows.h>
#include <gdiplus.h>
#include <commctrl.h>
#include "logControl.hpp"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

constexpr wchar_t szClassName[] = L"TcpClient";

constexpr int initialWidth = 1280,
    initialHeight = 720;

HINSTANCE hInstanceMain;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpszArgument, int nCmdShow)
{
    WSADATA wsaData;

    constexpr WORD wVersionRequested = MAKEWORD(2, 2);

    int iResult = WSAStartup(wVersionRequested, &wsaData);
    if (iResult != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d\n", iResult);
        return 1;
    }

    INITCOMMONCONTROLSEX initCtrls;

    initCtrls.dwSize = sizeof(initCtrls);
    initCtrls.dwICC = ICC_UPDOWN_CLASS;

    InitCommonControlsEx(&initCtrls);

    using Gdiplus::GdiplusStartupInput;
    using Gdiplus::GdiplusShutdown;

    HWND hwnd;
    WNDCLASSEXW wincl;

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;

    hInstanceMain = hInstance;

    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    wincl.hInstance = hInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WndProc;
    wincl.style = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wincl.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wincl.lpszMenuName = nullptr;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;

    wincl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

    if (!RegisterClassExW(&wincl))
    {
        return 0;
    }

    RegisterLogControl(hInstance);

    hwnd = CreateWindowExW(
           0,
           szClassName,
           szClassName,
           WS_OVERLAPPEDWINDOW,
           CW_USEDEFAULT,
           CW_USEDEFAULT,
           initialWidth,
           initialHeight,
           HWND_DESKTOP,
           nullptr,
           hInstance,
           nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);

    if (WSACleanup() != 0)
    {
        iResult = WSAGetLastError();
        fprintf(stderr,"WSACleanup failed: %d\n", iResult);
        return 1;
    }

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        OnCreate(hwnd, hInstanceMain);
        break;

    case WM_SIZE:
        OnResize(hwnd, wParam, LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_PAINT:
        OnPaint(hwnd);
        break;

    case WM_COMMAND:
        OnCommand(hwnd, wParam, lParam);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    return 0;
}
