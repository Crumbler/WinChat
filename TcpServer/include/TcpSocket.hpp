#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

class TcpSocket
{
    public:
        TcpSocket();
        TcpSocket(SOCKET sock);
        ~TcpSocket();

        void Bind(const char *addr, const char *port);
        void Listen(int backlog);
        TcpSocket* Accept();

        void Shutdown(int how);
        int Receive(char *buf, int length, int flags);
        SOCKET getSocket();
        void ShareProperties(TcpSocket& s);

    protected:

    private:
        SOCKET sock;
};
