#include "windowFuncs.hpp"

#include <cstdio>

#include <gdiplus.h>

#include <windowsx.h>
#include "logControl.hpp"
#include "TcpSocket.hpp"
#include "MessageType.hpp"

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

constexpr int bufSize = 256;

TcpSocket *sock;
int clientWidth, clientHeight;
bool isConnected;
char currUsername[bufSize];

HWND hSendButton, hEditMsg, hEditAddress,
    hEditPort, hEditUsername, hLog,
    hConnectButton, hDisconnectButton,
    hMainWindow;

void CalculateControlPositions();
void SetConnected(bool isConnected);
void OnConnect();
void OnDisconnect();
void OnClear();
void OnSend();
unsigned __stdcall ListenThread(void *param);

void AddToLog(const char *s)
{
    SendMessage(hLog, WM_LOGADD, (WPARAM)s, 0);
}

void OnCreate(HWND hwnd, HINSTANCE hInstance)
{
    hMainWindow = hwnd;

    hConnectButton = CreateWindowW(L"BUTTON", L"Connect",
                                   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   padding, 10, panelWidth - padding * 2, 30, hwnd,
                                   (HMENU)idBtnConnect, hInstance, nullptr);

    CreateWindowW(L"STATIC", L"Server IP address:",
                  WS_CHILD | WS_VISIBLE | SS_LEFT,
                  padding, 50, panelWidth - padding, 20, hwnd,
                  (HMENU)idLabel1, hInstance, nullptr);

    hEditAddress = CreateWindowW(L"EDIT", L"",
                                 WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
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

    Edit_LimitText(hEditPort, 5);

    CreateWindowW(L"STATIC", L"Username:",
                  WS_CHILD | WS_VISIBLE | SS_LEFT,
                  padding, 150, panelWidth - padding, 20, hwnd,
                  (HMENU)idLabel3, hInstance, nullptr);

    hEditUsername = CreateWindowW(L"EDIT", L"",
                                  WS_CHILD | WS_VISIBLE,
                                  padding, 170, panelWidth - padding * 2, 20, hwnd,
                                  (HMENU)idEditUsername, hInstance, nullptr);

    hDisconnectButton = CreateWindowW(L"BUTTON", L"Disconnect",
                                      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                      padding, 200, panelWidth - padding * 2, 30, hwnd,
                                      (HMENU)idBtnDisconnect, hInstance, nullptr);

    CreateWindowW(L"BUTTON", L"Clear chat log",
                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  padding, 240, panelWidth - padding * 2, 30, hwnd,
                  (HMENU)idBtnClear, hInstance, nullptr);

    hSendButton = CreateWindowW(L"BUTTON", L"Send",
                                WS_CHILD | WS_VISIBLE,
                                0, 0, sendBtnWidth, bottomPanelHeight - padding * 2, hwnd,
                                (HMENU)idBtnSend, hInstance, nullptr);

    hEditMsg = CreateWindowW(L"EDIT", L"",
                             WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                             0, 0, 0, 0, hwnd,
                             (HMENU)idEditMsg, hInstance, nullptr);

    Edit_LimitText(hEditMsg, 200);

    hLog = CreateWindowW(szLogControl, nullptr,
                         WS_CHILD | WS_VISIBLE | WS_VSCROLL,
                         0, 0, 0, 0, hwnd,
                         (HMENU)idLog, hInstance, nullptr);

    isConnected = false;
    SetConnected(isConnected);

    sock = new TcpSocket();
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

void SetConnected(bool isConnected)
{
    EnableWindow(hConnectButton, !isConnected);
    EnableWindow(hDisconnectButton, isConnected);
    EnableWindow(hSendButton, isConnected);
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
    char addr[64], port[64], username[64];

    Edit_GetText(hEditAddress, addr, 64);
    Edit_GetText(hEditPort, port, 64);
    Edit_GetText(hEditUsername, username, 64);

    int iResult = sock->Connect(addr, port);

    if (iResult != 0)
    {
        char error[64];
        sprintf_s(error, sizeof(error), "Can't connect to the server, winsock error: %d", iResult);
        AddToLog(error);
        return;
    }

    // Account for null-terminating character
    const int usernameLength = strlen(username) + 1;

    memcpy(currUsername, username, usernameLength);

    char *buf = new char[bufSize];

    buf[0] = MessageType::NameOffer;
    buf[1] = usernameLength;
    memcpy(buf + 2, username, usernameLength);

    sock->Send(buf, usernameLength + 2);

    sock->Receive(buf, 1, 0);

    MessageType res = (MessageType)buf[0];

    // Rejected
    if (res == MessageType::NameTaken)
    {
        sprintf_s(buf, bufSize, "Can't connect to the server, name %s already in use", username);
        AddToLog(buf);

        delete[] buf;

        sock->Shutdown(SD_BOTH);
        sock->ReCreate();

        return;
    }

    AddToLog("Successfully connected");

    delete[] buf;

    isConnected = true;
    SetConnected(isConnected);

    // Start listening
    _beginthreadex(nullptr, 0, ListenThread, nullptr, 0, nullptr);
}

void OnDisconnect()
{
    if (!isConnected)
    {
        return;
    }

    sock->Shutdown(SD_BOTH);

    AddToLog("Disconnected from server");

    isConnected = false;
    SetConnected(isConnected);
    sock->ReCreate();
}

// Called in response to a WM_ADDTOLOG message from the listen thread
void OnAddToLog(char *s)
{
    AddToLog(s);

    delete[] s;
}

void OnClear()
{
    SendMessage(hLog, WM_LOGCLEAR, 0, 0);
}

void OnSend()
{
    char *sendBuf = new char[bufSize];

    sendBuf[0] = MessageType::ClientMessage;

    Edit_GetText(hEditMsg, sendBuf + 2, bufSize - 2);

    // Account for null-terminating character
    int len = strlen(sendBuf + 2) + 1;

    sendBuf[1] = len;

    sock->Send(sendBuf, len + 2);

    Edit_SetText(hEditMsg, nullptr);

    delete[] sendBuf;
}

unsigned __stdcall ListenThread(void *param)
{
    printf("Started listen thread\n");

    char *buf = new char[bufSize];

    while (true)
    {
        int bytesToReceive = 3, iResult;
        char *bufPos = buf;

        while (bytesToReceive > 0)
        {
            iResult = sock->Receive(bufPos, bytesToReceive, 0);
            if (iResult == SOCKET_ERROR || iResult == 0)
            {
                PostMessage(hMainWindow, WM_DISCONNECTED, 0, 0);

                goto breakLoop;
            }

            bufPos += iResult;
            bytesToReceive -= iResult;
        }

        const int lengthName = (unsigned char)buf[1],
            lengthMsg = (unsigned char)buf[2];

        bytesToReceive = lengthName + lengthMsg;
        while (bytesToReceive > 0)
        {
            iResult = sock->Receive(bufPos, bytesToReceive, 0);
            if (iResult == SOCKET_ERROR || iResult == 0)
            {
                PostMessage(hMainWindow, WM_DISCONNECTED, 0, 0);

                goto breakLoop;
            }

            bufPos += iResult;
            bytesToReceive -= iResult;
        }

        // Disposed upon processing the message WM_ADDTOLOG
        char *buf2 = new char[bufSize];

        // If the current user is the sender, replace the username
        if (strcmp(currUsername, buf + 3) == 0)
        {
            strcpy(buf + 3, "me");
        }

        sprintf_s(buf2, bufSize, "%s: %s", buf + 3, buf + lengthName + 3);

        PostMessage(hMainWindow, WM_ADDTOLOG, (WPARAM)buf2, 0);
    }

breakLoop:

    delete[] buf;

    return 0;
}
