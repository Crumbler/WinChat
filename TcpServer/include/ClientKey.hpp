#pragma once

#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>
#include "TcpSocket.hpp"
#include "IOOverlapped.hpp"
#include "MessageType.hpp"

struct ClientKey
{
    TcpSocket *socket;
    WSABUF inBuf, outBuf;
    char *inBufBase, *outBufBase;
    IOOverlapped ovIn, ovOut;

    std::string *username;
    MessageType lastOutMsgType;

    int bytesExpected, bytesReceived;

    ClientKey(TcpSocket *socket, int bufSize);

    void ReceiveAsync(int bytes);
    void SendAsync(int bytes);
    void AdvanceInputBuffer(int bytes);
    void ResetInput();

    ~ClientKey();
};
