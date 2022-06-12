#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

class TcpSocket
{
    public:
        TcpSocket();
        TcpSocket(SOCKET sock);
        ~TcpSocket();

        void Bind(const wchar_t *addr, const wchar_t *port);
        void Listen(int backlog);
        TcpSocket* Accept();
        void Shutdown(int how);
        int Receive(char *buf, int length, int flags);
        bool Connect(const wchar_t *addr, const wchar_t *port);

    protected:

    private:
        SOCKET sock;
};
