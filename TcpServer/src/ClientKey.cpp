#include "ClientKey.hpp"
#include "IOType.hpp"

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

    this->ResetInput();
}

void ClientKey::ReceiveAsync(int bytes)
{
    this->ovIn->hEvent = (HANDLE)IOType::Receive;

    this->inBuf.len = bytes;

    DWORD bytesReceived, flags = 0;
    int iResult = WSARecv(this->socket->getSocket(), &this->inBuf, 1,
                          &bytesReceived, &flags, this->ovIn, nullptr);

    if (iResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        fprintf(stderr, "WSARecv() error:%d\n", WSAGetLastError());
    }
}

void ClientKey::AdvanceInputBuffer(int bytes)
{
    this->inBuf.buf += bytes;
}

void ClientKey::ResetInput()
{
    this->bytesExpected = 0;
    this->bytesReceived = 0;
    this->inBuf.buf = this->inBufBase;
}

ClientKey::~ClientKey()
{
    delete this->inBufBase;
    delete this->outBufBase;
    delete this->ovIn;
    delete this->ovOut;
    delete this->socket;
}
