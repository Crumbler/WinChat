
#include <cstdio>

#include <stdexcept>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "ClientKey.hpp"
#include "TcpSocket.hpp"
#include "WorkerThread.hpp"
#include "PipeThread.hpp"
#include "MessageType.hpp"
#include "IOType.hpp"
#include "IOOverlapped.hpp"

constexpr int socketBufSize = 256,
    listenQueueSize = 10;
constexpr wchar_t serverPort[] = L"27000";

HANDLE cmpPort;
CRITICAL_SECTION clientsSection;

int main()
{
    WSADATA wsaData;
    SYSTEM_INFO systemInfo;

    GetSystemInfo(&systemInfo);

    const int threadCount = systemInfo.dwNumberOfProcessors * 2;

    constexpr WORD wVersionRequested = MAKEWORD(2, 2);

    // No console output buffering
    setbuf(stderr, nullptr);
    setbuf(stdout, nullptr);

    int iResult = WSAStartup(wVersionRequested, &wsaData);
    if (iResult != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d\n", iResult);
        return 1;
    }

    constexpr int spinCount = 4000;
    InitializeCriticalSectionAndSpinCount(&clientsSection, spinCount);

    cmpPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
                                     nullptr, 0, threadCount);

    for (int i = 0; i < threadCount; ++i)
    {
        _beginthreadex(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
    }

    _beginthreadex(nullptr, 0, PipeThread, nullptr, 0, nullptr);

    TcpSocket* listenSocket = new TcpSocket();

    listenSocket->Bind(nullptr, serverPort);
    listenSocket->Listen(listenQueueSize);

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
            delete key;
            continue;
        }

        key->bytesExpected = 2;
        key->ReceiveAsync(2);
    }

    delete listenSocket;

    CloseHandle(cmpPort);

    DeleteCriticalSection(&clientsSection);

    if (WSACleanup() != 0)
    {
        iResult = WSAGetLastError();
        fprintf(stderr,"WSACleanup failed: %d\n", iResult);
        return 1;
    }

    return 0;
}
