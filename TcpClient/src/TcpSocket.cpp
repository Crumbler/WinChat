#include "TcpSocket.hpp"

#include <cstdio>

TcpSocket::TcpSocket()
{
    this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (this->sock == INVALID_SOCKET)
    {
        fprintf(stderr, "Error at socket(): %d\n", WSAGetLastError());
    }
}

TcpSocket::TcpSocket(SOCKET sock)
{
    this->sock = sock;
}

TcpSocket::~TcpSocket()
{
    if (this->sock != INVALID_SOCKET)
    {
        closesocket(this->sock);
    }
}

void TcpSocket::Bind(const wchar_t *addr, const wchar_t *port)
{
    ADDRINFOW hints, *result;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int iResult = GetAddrInfoW(addr, port, &hints, &result);
    if (iResult != 0)
    {
        fprintf(stderr, "getaddrinfo failed: %d\n", iResult);
        return;
    }

    iResult = bind(this->sock, result->ai_addr, result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        fprintf(stderr, "Error at bind(): %d\n", WSAGetLastError());
    }

    FreeAddrInfoW(result);
}

void TcpSocket::Listen(int backlog)
{
    if (listen(this->sock, backlog) == SOCKET_ERROR)
    {
        fprintf(stderr, "Error at listen(): %d\n", WSAGetLastError());
        closesocket(this->sock);
        this->sock = INVALID_SOCKET;
    }
}

int TcpSocket::Send(const char* buf, int length)
{
    int iResult = send(this->sock, buf, length, 0);
    if (iResult == SOCKET_ERROR)
    {
        fprintf(stderr, "Error at send(): %d\n", WSAGetLastError());
        return -1;
    }

    return iResult;
}



TcpSocket* TcpSocket::Accept()
{
    SOCKET sock = accept(this->sock, nullptr, nullptr);
    if (sock == INVALID_SOCKET)
    {
        fprintf(stderr, "accept failed: %d\n", WSAGetLastError());
        return nullptr;
    }

    return new TcpSocket(sock);
}

void TcpSocket::Shutdown(int how)
{
    int iResult = shutdown(this->sock, how);
    if (iResult != 0)
    {
        fprintf(stderr, "shutdown failed: %d\n", WSAGetLastError());
    }
}

int TcpSocket::Receive(char* buf, int length, int flags)
{
    return recv(this->sock, buf, length, flags);
}

bool TcpSocket::Connect(const wchar_t* addr, const wchar_t* port)
{
    ADDRINFOW hints, *result;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int iResult = GetAddrInfoW(addr, port, &hints, &result);
    if (iResult != 0)
    {
        fprintf(stderr, "getaddrinfo failed: %d\n", iResult);
        return false;
    }

    iResult = connect(this->sock, result->ai_addr, result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        fprintf(stderr, "Error at connect(): %d\n", WSAGetLastError());
    }

    FreeAddrInfoW(result);

    return iResult == 0;
}
