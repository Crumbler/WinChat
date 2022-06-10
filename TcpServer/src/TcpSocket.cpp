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

TcpSocket::~TcpSocket()
{
    closesocket(this->sock);
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
        FreeAddrInfoW(result);
        closesocket(this->sock);
    }

    FreeAddrInfoW(result);
}
