
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

    TcpSocket* socket = new TcpSocket();

    const bool res = socket->Connect(L"127.0.0.1", L"27000");

    if (res)
    {
        printf("Got connection\n");

        char sendBuf[256];

        for (int i = 1; i < 254; ++i)
        {
            sendBuf[i] = 'n';
        }
        sendBuf[254] = 'v';
        sendBuf[255] = 0;

        int len = 255;
        sendBuf[0] = len;
        printf("Sending message of length: %d\n", len);

        int bytesSent = socket->Send(sendBuf, len + 1);

        printf("Sent %d bytes\n", bytesSent);

        socket->Shutdown(SD_BOTH);
    }

    getchar();

    delete socket;

    if (WSACleanup() != 0)
    {
        iResult = WSAGetLastError();
        fprintf(stderr,"WSACleanup failed: %d\n", iResult);
        return 1;
    }

    return 0;
}
