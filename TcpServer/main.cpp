
#include <cstdio>

#include <winsock2.h>
#include <ws2tcpip.h>

int main()
{
    WSADATA wsaData;

    constexpr WORD wVersionRequested = MAKEWORD(2, 2);

    int iResult = WSAStartup(wVersionRequested, &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    ADDRINFOW hints, *result;

    ZeroMemory(&hints, sizeof(hints));

    iResult = GetAddrInfoW(L"127.0.0.1", L"27000", &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed: %d\n", iResult);
        return 1;
    }

    if (WSACleanup() != 0)
    {
        iResult = WSAGetLastError();
        printf("WSACleanup failed: %d\n", iResult);
        return 1;
    }

    return 0;
}
