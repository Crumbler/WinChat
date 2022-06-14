
#include <cstdio>

#include <stdexcept>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "ClientKey.hpp"
#include "TcpSocket.hpp"
#include "IOType.hpp"

constexpr int socketBufSize = 256;

HANDLE cmpPort;

unsigned WorkerThread(void *param);

int main()
{
    WSADATA wsaData;
    SYSTEM_INFO systemInfo;

    GetSystemInfo(&systemInfo);

    const int threadCount = systemInfo.dwNumberOfProcessors * 2;

    constexpr WORD wVersionRequested = MAKEWORD(2, 2);

    setbuf(stderr, nullptr);
    setbuf(stdout, nullptr);

    int iResult = WSAStartup(wVersionRequested, &wsaData);
    if (iResult != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d\n", iResult);
        return 1;
    }

    cmpPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
                                     nullptr, 0, threadCount);

    for (int i = 0; i < threadCount; ++i)
    {
        _beginthreadex(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
    }

    TcpSocket* listenSocket = new TcpSocket();

    listenSocket->Bind(nullptr, L"27000");
    listenSocket->Listen(10);

    // Keep accepting connections
    while (true)
    {
        TcpSocket *clientSocket = listenSocket->Accept();
        printf("Got connection\n");

        ClientKey *key = new ClientKey(clientSocket, socketBufSize);

        HANDLE hRes = CreateIoCompletionPort((HANDLE)clientSocket->getSocket(),
            cmpPort, (ULONG_PTR)key, 0);

        if (hRes == nullptr)
        {
            fprintf(stderr, "Addition to port failed:%lu\n", GetLastError());
        }

        key->bytesExpected = 2;
        key->ReceiveAsync(2);
    }

    delete listenSocket;

    CloseHandle(cmpPort);

    if (WSACleanup() != 0)
    {
        iResult = WSAGetLastError();
        fprintf(stderr,"WSACleanup failed: %d\n", iResult);
        return 1;
    }

    return 0;
}

unsigned __stdcall WorkerThread(void *param)
{
    printf("Thread started\n");

    DWORD trBytes;
    ClientKey *pKey;
    OVERLAPPED *pOv;

    while (true)
    {
        WINBOOL iResult =
            GetQueuedCompletionStatus(cmpPort, &trBytes, (PULONG_PTR)&pKey, &pOv, INFINITE);

        if (iResult == 0)
        {
            fprintf(stderr, "Receive error: %lu\n", GetLastError());
            continue;
        }
        else if (iResult != 0 && trBytes == 0)
        {
            printf("Client disconnected\n");
            continue;
        }

        IOType ioType = (IOType)pKey->inBufBase[0];

        if (ioType == IOType::Send)
        {
            continue;
        }

        // Incomplete receival
        pKey->bytesReceived += trBytes;
        const int bytesLeftToReceive = pKey->bytesExpected - pKey->bytesReceived;
        if (bytesLeftToReceive > 0)
        {
            printf("Received incomplete message with %d missing bytes\n", bytesLeftToReceive);
            pKey->AdvanceInputBuffer(trBytes);
            pKey->ReceiveAsync(bytesLeftToReceive);
            continue;
        }

        int msgType = pKey->inBufBase[0];
        int msgLength = pKey->inBufBase[1];

        // If message type and length have been received
        if (pKey->bytesReceived == 2)
        {
            printf("Receiving message of type %d and length %d\n", msgType, msgLength);

            // Receive the message itself
            pKey->AdvanceInputBuffer(2);
            pKey->ReceiveAsync(msgLength);
            continue;
        }

        printf("Received a %d character message:\n%s\n", msgLength, pKey->inBufBase + 2);

        pKey->ResetInput();
        pKey->bytesExpected = 2;
        pKey->ReceiveAsync(2);
    }
}
