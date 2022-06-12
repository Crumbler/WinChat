#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include "TcpSocket.hpp"

struct ClientKey
{
    TcpSocket *socket;
    WSABUF inBuf, outBuf;
    OVERLAPPED *overlapped;

    ClientKey(TcpSocket *socket, int bufSize);

    void ReceiveAsync();

    ~ClientKey();
};
