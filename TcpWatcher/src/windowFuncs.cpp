#include "windowFuncs.hpp"

#include <cstdio>

#include <gdiplus.h>

#include "logControl.hpp"

using namespace Gdiplus;

constexpr int idBtnFetch = 0, idLog = 1;

constexpr int panelWidth = 80,
    padding = 10;

constexpr int bufSize = 256;

int clientWidth, clientHeight;

HWND hFetchButton, hLog;

void CalculateControlPositions();
void OnFetch();

void ClearLog()
{
    SendMessage(hLog, WM_LOGCLEAR, 0, 0);
}

void AddToLog(const char *s)
{
    SendMessage(hLog, WM_LOGADD, (WPARAM)s, 0);
}

void OnCreate(HWND hwnd, HINSTANCE hInstance)
{
    hFetchButton = CreateWindowW(L"BUTTON", L"Fetch",
                                 WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                 padding, 10, panelWidth - padding * 2, 30, hwnd,
                                 (HMENU)idBtnFetch, hInstance, nullptr);

    hLog = CreateWindowW(szLogControl, nullptr,
                         WS_CHILD | WS_VISIBLE | WS_VSCROLL,
                         0, 0, 0, 0, hwnd,
                         (HMENU)idLog, hInstance, nullptr);
}

void OnResize(HWND hwnd, int resizeType, int newWidth, int newHeight)
{
    clientWidth = newWidth;
    clientHeight = newHeight;

    CalculateControlPositions();
}

void CalculateControlPositions()
{
    UINT flags = SWP_NOZORDER;
    SetWindowPos(hLog, nullptr, panelWidth, 0, clientWidth - panelWidth,
                 clientHeight, flags);
}

void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    SolidBrush brush(Color(240, 240, 240));

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
    switch (idControl)
    {
    case idBtnFetch:
        if (idNotify == BN_CLICKED)
        {
            OnFetch();
        }
        break;
    }
}

void OnFetch()
{
    char *buf = new char[bufSize],
        *strBuf = new char[bufSize];

    ClearLog();

    HANDLE hPipe = CreateFileA("\\\\.\\pipe\\tcpwatcher", GENERIC_READ, 0, nullptr,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        const DWORD errorCode = GetLastError();

        char buf[64];
        sprintf_s(buf, bufSize, "Open pipe error: %lu", errorCode);

        AddToLog(buf);

        return;
    }

    DWORD trBytes;

    // Receive number of clients
    ReadFile(hPipe, buf, 2, &trBytes, nullptr);

    int nClients = *(short *)buf;

    sprintf_s(strBuf, bufSize, "Fetched %d clients", nClients);
    AddToLog(strBuf);

    for (int i = 0; i < nClients; ++i)
    {
        // Receive client name length including the null-terminating character
        ReadFile(hPipe, buf, 1, &trBytes, nullptr);

        int bytesToReceive = (unsigned char)buf[0];

        ReadFile(hPipe, buf, bytesToReceive, &trBytes, nullptr);

        AddToLog(buf);
    }

    CloseHandle(hPipe);

    delete[] strBuf;
    delete[] buf;
}
