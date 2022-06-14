
#include <cstdio>

#include <stdexcept>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "TcpSocket.hpp"

#include "MessageType.hpp"

constexpr int usrBufSize = 64, bufSize = 256;

void OnConnected();
bool ConfirmName();
void SendMessages();

TcpSocket *sock;

char username[usrBufSize];

// Length accounting for the null-terminating character
int usernameLength;

int main()
{
    WSADATA wsaData;

    constexpr WORD wVersionRequested = MAKEWORD(2, 2);

    int iResult = WSAStartup(wVersionRequested, &wsaData);
    if (iResult != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d\n", iResult);
        return 1;
    }

    sock = new TcpSocket();

    printf("Enter your username:\n");

    fgets(username, usrBufSize, stdin);

    // Overwrite the newline character
    usernameLength = strlen(username);
    username[usernameLength - 1] = 0;

    const bool res = sock->Connect(L"127.0.0.1", L"27000");

    if (res)
    {
        OnConnected();
    }

    getchar();

    delete sock;

    if (WSACleanup() != 0)
    {
        iResult = WSAGetLastError();
        fprintf(stderr,"WSACleanup failed: %d\n", iResult);
        return 1;
    }

    return 0;
}

void OnConnected()
{
    printf("Got connection\n");

    if (ConfirmName())
    {
        SendMessages();
    }

    sock->Shutdown(SD_BOTH);
}

bool ConfirmName()
{
    char *buf = new char[bufSize];

    buf[0] = MessageType::NameOffer;
    buf[1] = usernameLength;

    memcpy(buf + 2, username, usernameLength);

    sock->Send(buf, usernameLength + 2);

    printf("Sent name\n");

    // Receive name approval or disapproval
    sock->Receive(buf, 1, 0);

    MessageType res = (MessageType)buf[0];

    if (res != MessageType::NameAccepted)
    {
        printf("The username %s is already taken\n", username);
    }
    else
    {
        printf("The username %s has been accepted\n", username);
    }

    delete[] buf;

    return res == MessageType::NameAccepted;
}

void SendMessages()
{
    char *buf = new char[bufSize];

    buf[0] = MessageType::ClientMessage;

    printf("Enter messages:\n");

    while (true)
    {
        fgets(buf + 2, bufSize - 2, stdin);

        // Overwrite the newline character
        int len = strlen(buf + 2);
        buf[len + 2 - 1] = 0;

        buf[1] = len;

        sock->Send(buf, len + 2);
    }

    delete[] buf;
}
