#include "windowFuncs.hpp"

#include <cstdio>

#include <gdiplus.h>

#include "logControl.hpp"

using namespace Gdiplus;

constexpr int idLabel1 = 0,
    idBtnConnect = 1, idBtnDisconnect = 2,
    idBtnClear = 3, idEditAddress = 4, idEditPort = 5,
    idEditUsername = 6, idBtnSend = 7, idEditMsg = 8,
    idLabel2 = 9, idLabel3 = 10, idLog = 11;

constexpr int panelWidth = 140,
    bottomPanelHeight = 50,
    padding = 10,
    sendBtnWidth = 60;

int clientWidth, clientHeight;

HWND hSendButton, hEditMsg, hEditAddress,
    hEditPort, hEditUsername, hLog;

void CalculateControlPositions();
void OnConnect();
void OnDisconnect();
void OnClear();
void OnSend();

void OnCreate(HWND hwnd, HINSTANCE hInstance)
{
    CreateWindowW(L"BUTTON", L"Connect",
                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  padding, 10, panelWidth - padding * 2, 30, hwnd,
                  (HMENU)idBtnConnect, hInstance, nullptr);

    CreateWindowW(L"STATIC", L"Server IP address:",
                  WS_CHILD | WS_VISIBLE | SS_LEFT,
                  padding, 50, panelWidth - padding, 20, hwnd,
                  (HMENU)idLabel1, hInstance, nullptr);

    hEditAddress = CreateWindowW(L"EDIT", L"",
                                 WS_CHILD | WS_VISIBLE,
                                 padding, 70, panelWidth - padding * 2, 20, hwnd,
                                 (HMENU)idEditAddress, hInstance, nullptr);

    CreateWindowW(L"STATIC", L"Server port:",
                  WS_CHILD | WS_VISIBLE | SS_LEFT,
                  padding, 100, panelWidth - padding, 20, hwnd,
                  (HMENU)idLabel2, hInstance, nullptr);

    hEditPort = CreateWindowW(L"EDIT", L"",
                              WS_CHILD | WS_VISIBLE,
                              padding, 120, panelWidth - padding * 2, 20, hwnd,
                              (HMENU)idEditPort, hInstance, nullptr);

    CreateWindowW(L"STATIC", L"Username:",
                  WS_CHILD | WS_VISIBLE | SS_LEFT,
                  padding, 150, panelWidth - padding, 20, hwnd,
                  (HMENU)idLabel3, hInstance, nullptr);

    hEditUsername = CreateWindowW(L"EDIT", L"",
                                  WS_CHILD | WS_VISIBLE,
                                  padding, 170, panelWidth - padding * 2, 20, hwnd,
                                  (HMENU)idEditUsername, hInstance, nullptr);

    CreateWindowW(L"BUTTON", L"Disconnect",
                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  padding, 200, panelWidth - padding * 2, 30, hwnd,
                  (HMENU)idBtnDisconnect, hInstance, nullptr);

    CreateWindowW(L"BUTTON", L"Clear chat log",
                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  padding, 240, panelWidth - padding * 2, 30, hwnd,
                  (HMENU)idBtnClear, hInstance, nullptr);

    hSendButton = CreateWindowW(L"BUTTON", L"Send",
                                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                0, 0, sendBtnWidth, bottomPanelHeight - padding * 2, hwnd,
                                (HMENU)idBtnSend, hInstance, nullptr);

    hEditMsg = CreateWindowW(L"EDIT", L"",
                             WS_CHILD | WS_VISIBLE,
                             0, 0, 0, 0, hwnd,
                             (HMENU)idEditMsg, hInstance, nullptr);

    hLog = CreateWindowW(szLogControl, nullptr,
                         WS_CHILD | WS_VISIBLE | WS_VSCROLL,
                         0, 0, 0, 0, hwnd,
                         (HMENU)idLog, hInstance, nullptr);

    char arr[] = "   Test message";

    for (int i = 0; i < 30; ++i)
    {
        arr[0] = '0' + i / 10;
        arr[1] = '0' + i % 10;

        SendMessage(hLog, WM_LOGADD, (WPARAM)arr, 0);
    }
}

void OnResize(HWND hwnd, int resizeType, int newWidth, int newHeight)
{
    clientWidth = newWidth;
    clientHeight = newHeight;

    CalculateControlPositions();
}

void CalculateControlPositions()
{
    UINT flags = SWP_NOZORDER | SWP_NOSIZE;
    SetWindowPos(hSendButton, nullptr, padding, clientHeight - bottomPanelHeight + padding, 0, 0, flags);

    flags = SWP_NOZORDER;
    SetWindowPos(hEditMsg, nullptr, sendBtnWidth + padding * 2, clientHeight - bottomPanelHeight + padding,
                 clientWidth - sendBtnWidth - padding * 3, bottomPanelHeight - padding * 2, flags);

    SetWindowPos(hLog, nullptr, panelWidth, 0, clientWidth - panelWidth,
                 clientHeight - bottomPanelHeight, flags);
}

void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    SolidBrush brush(Color(240, 240, 240));

    graphics.FillRectangle(&brush, 0, 0, panelWidth, clientHeight - bottomPanelHeight);

    graphics.FillRectangle(&brush, 0, clientHeight - bottomPanelHeight, clientWidth, bottomPanelHeight);

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
    case idBtnConnect:
        if (idNotify == BN_CLICKED)
        {
            OnConnect();
        }
        break;

    case idBtnDisconnect:
        if (idNotify == BN_CLICKED)
        {
            OnDisconnect();
        }
        break;

    case idBtnClear:
        if (idNotify == BN_CLICKED)
        {
            OnClear();
        }
        break;

    case idBtnSend:
        if (idNotify == BN_CLICKED)
        {
            OnSend();
        }
        break;
    }
}

void OnConnect()
{

}

void OnDisconnect()
{

}

void OnClear()
{
    SendMessage(hLog, WM_LOGCLEAR, 0, 0);
}

void OnSend()
{

}
