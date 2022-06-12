#include "ClientKey.hpp"

#include <cstdio>

ClientKey::ClientKey(TcpSocket *socket, int bufSize)
{
    this->socket = socket;

    inBuf.buf = new char[bufSize];
    inBuf.len = bufSize;

    outBuf.buf = new char[bufSize];
    outBuf.len = bufSize;

    this->overlapped = new OVERLAPPED();
}

void ClientKey::ReceiveAsync()
{
    DWORD bytes, flags = 0;
    int iResult = WSARecv(this->socket->getSocket(), &this->inBuf, 1,
                          &bytes, &flags, this->overlapped, nullptr);

    if (iResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        fprintf(stderr, "WSARecv() error:%d\n", WSAGetLastError());
    }
}

ClientKey::~ClientKey()
{
    delete this->inBuf.buf;
    delete this->outBuf.buf;
    delete this->overlapped;
    delete this->socket;
}
