#include "PipeThread.hpp"

#include <cstdio>
#include <unordered_map>
#include "ClientKey.hpp"

struct strPtrHash
{
    unsigned operator()(std::string *s) const
    {
        return std::hash<std::string>()(*s);
    }
};

struct strPtrEqual
{
    unsigned operator()(std::string *s1, std::string *s2) const
    {
        return *s1 == *s2;
    }
};

constexpr int bufSize = 256;

extern CRITICAL_SECTION clientsSection;
extern std::unordered_map<std::string*, ClientKey*, strPtrHash, strPtrEqual> clients;

unsigned __stdcall PipeThread(void *param)
{
    printf("Pipe thread started\n");

    HANDLE hPipe = CreateNamedPipeA("\\\\.\\pipe\\tcpwatcher", PIPE_ACCESS_OUTBOUND,
                     PIPE_TYPE_BYTE | PIPE_WAIT, 1, bufSize, 0, 0, nullptr);

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Pipe creation error: %lu", GetLastError());
        return 0;
    }

    while (true)
    {
        int iResult = ConnectNamedPipe(hPipe, nullptr);
        if (iResult == 0)
        {
            fprintf(stderr, "Pipe connection failed: %lu", GetLastError());
            continue;
        }

        DWORD trBytes;

        EnterCriticalSection(&clientsSection);

        const short nClients = clients.size();
        WriteFile(hPipe, &nClients, 2, &trBytes, nullptr);

        for (auto entry : clients)
        {
            const std::string *username = entry.first;

            // Account for null-terminating character
            const unsigned char len = username->size() + 1;

            // Send username length
            WriteFile(hPipe, &len, 1, &trBytes, nullptr);

            // Send the username itself
            WriteFile(hPipe, username->data(), len, &trBytes, nullptr);
        }

        FlushFileBuffers(hPipe);

        LeaveCriticalSection(&clientsSection);

        DisconnectNamedPipe(hPipe);
    }

    CloseHandle(hPipe);

    return 0;
}
