
#include <cstdio>

#include <stdexcept>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "TcpSocket.hpp"

int main()
{
    WSADATA wsaData;

    constexpr WORD wVersionRequested = MAKEWORD(2, 2);

    int iResult = WSAStartup(wVersionRequested, &wsaData);
    if (iResult != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d\n", iResult);
        return 1;
    }

    TcpSocket* listenSocket = new TcpSocket();

    listenSocket->Bind(nullptr, L"27000");
    listenSocket->Listen(10);
    TcpSocket *clientSocket = listenSocket->Accept();

    char *buf = new char[512];

    printf("Got connection\n");

    listenSocket->Shutdown(SD_BOTH);

    delete clientSocket;
    delete[] buf;
    delete listenSocket;

    if (WSACleanup() != 0)
    {
        iResult = WSAGetLastError();
        fprintf(stderr,"WSACleanup failed: %d\n", iResult);
        return 1;
    }

    return 0;
}
