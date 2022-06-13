#include "ClientKey.hpp"

#include <cstdio>

ClientKey::ClientKey(TcpSocket *socket, int bufSize)
{
    this->socket = socket;

    this->inBufBase = new char[bufSize];
    this->inBuf.buf = this->inBufBase;
    this->inBuf.len = bufSize;

    this->outBufBase = new char[bufSize];
    this->outBuf.buf = this->outBufBase;
    this->outBuf.len = bufSize;

    this->ovIn = new OVERLAPPED();
    this->ovOut = new OVERLAPPED();
}

void ClientKey::ReceiveAsync()
{
    DWORD bytes, flags = 0;
    int iResult = WSARecv(this->socket->getSocket(), &this->inBuf, 1,
                          &bytes, &flags, this->ovIn, nullptr);

    if (iResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        fprintf(stderr, "WSARecv() error:%d\n", WSAGetLastError());
    }
}

ClientKey::~ClientKey()
{
    delete this->inBufBase;
    delete this->outBufBase;
    delete this->ovIn;
    delete this->ovOut;
    delete this->socket;
}
