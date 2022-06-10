#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

class TcpSocket
{
    public:
        TcpSocket();
        ~TcpSocket();

        void Bind(const wchar_t *addr, const wchar_t *port);

    protected:

    private:
        SOCKET sock;
};
