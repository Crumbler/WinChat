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
        void ReCreate();
        int Send(const char *buf, int length);
        TcpSocket* Accept();
        void Shutdown(int how);
        int Receive(char *buf, int length, int flags);
        int Connect(const char *addr, const char *port);

    protected:

    private:
        SOCKET sock;
};
