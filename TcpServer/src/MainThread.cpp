
#include <cstdio>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <Mswsock.h>
#include "MainThread.hpp"

#include "ClientKey.hpp"
#include "TcpSocket.hpp"
#include "WorkerThread.hpp"
#include "PipeThread.hpp"
#include "MessageType.hpp"
#include "IOType.hpp"
#include "IOOverlapped.hpp"

constexpr int socketBufSize = 256,
    listenQueueSize = 10;
constexpr char serverPort[] = "27000";

HANDLE cmpPort, hPipeThread;
extern HANDLE hMainThread;
CRITICAL_SECTION clientsSection;
std::unordered_map<std::string*, ClientKey*, strPtrHash, strPtrEqual> clients;
bool stopMainThread = false;

void StopMainThread(ULONG_PTR p)
{
    stopMainThread = true;
}

unsigned _stdcall MainThread(void *param)
{
    SYSTEM_INFO systemInfo;

    GetSystemInfo(&systemInfo);

    const int threadCount = systemInfo.dwNumberOfProcessors * 2;

    HANDLE *workerThreads = new HANDLE[threadCount];

    constexpr int spinCount = 4000;
    InitializeCriticalSectionAndSpinCount(&clientsSection, spinCount);

    cmpPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
                                     nullptr, 0, threadCount);

    for (int i = 0; i < threadCount; ++i)
    {
        workerThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
    }

    hPipeThread = (HANDLE)_beginthreadex(nullptr, 0, PipeThread, nullptr, 0, nullptr);

    TcpSocket* listenSocket = new TcpSocket();

    listenSocket->Bind(nullptr, serverPort);
    listenSocket->Listen(listenQueueSize);

    DWORD dwBytes;
    LPFN_ACCEPTEX AcceptEx = nullptr;
    GUID GuidAcceptEx = WSAID_ACCEPTEX;

    // Get the pointer to AcceptEx
    int iResult = WSAIoctl(listenSocket->getSocket(), SIO_GET_EXTENSION_FUNCTION_POINTER,
                           &GuidAcceptEx, sizeof(GuidAcceptEx),
                           &AcceptEx, sizeof(AcceptEx),
                           &dwBytes, nullptr, nullptr);
    if (iResult == SOCKET_ERROR) {
        fprintf(stderr, "WSAIoctl failed with error: %d\n", WSAGetLastError());
    }

    OVERLAPPED ov;
    ZeroMemory(&ov, sizeof(ov));
    ov.hEvent = CreateEventA(nullptr, true, false, nullptr);

    char *buf = new char[128];

    // Keep accepting connections
    while (true)
    {
        TcpSocket *clientSocket = new TcpSocket();

        AcceptEx(listenSocket->getSocket(), clientSocket->getSocket(),
                 buf, 0, 64, 64, &dwBytes, &ov);

        iResult = WaitForSingleObjectEx(ov.hEvent, INFINITE, true);
        if (iResult == WAIT_IO_COMPLETION && stopMainThread)
        {
            printf("Stopping main thread\n");

            delete clientSocket;

            break;
        }

        // Inherit the properties of the listen socket
        listenSocket->ShareProperties(*clientSocket);

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

    QueueUserAPC(StopPipeThread, hPipeThread, (ULONG_PTR)nullptr);

    for (int i = 0; i < threadCount; ++i)
    {
        PostQueuedCompletionStatus(cmpPort, 0, (ULONG_PTR)nullptr, nullptr);
    }

    for (int i = 0; i < threadCount; ++i)
    {
        WaitForSingleObject(workerThreads[i], INFINITE);
        CloseHandle(workerThreads[i]);
    }

    delete[] workerThreads;

    WaitForSingleObject(hPipeThread, INFINITE);
    CloseHandle(hPipeThread);

    delete listenSocket;
    delete[] buf;

    CloseHandle(ov.hEvent);

    for (auto entry : clients)
    {
        ClientKey *key = entry.second;

        delete key;
    }

    clients.clear();

    CloseHandle(cmpPort);

    DeleteCriticalSection(&clientsSection);

    return 0;
}
