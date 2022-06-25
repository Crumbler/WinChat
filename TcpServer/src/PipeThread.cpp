#include <winsock2.h>
#include "PipeThread.hpp"
#include "WorkerThread.hpp"

#include <cstdio>
#include "ClientKey.hpp"
#include "ServerConfig.hpp"

constexpr int bufSize = 256;
bool stopPipeThread = false;
extern CRITICAL_SECTION clientsSection;

void StopPipeThread(ULONG_PTR p)
{
    stopPipeThread = true;
}

unsigned __stdcall PipeThread(void *param)
{
    printf("Pipe thread started\n");

    const ServerConfig *config = (const ServerConfig*)param;

    HANDLE hPipe = CreateNamedPipeA(config->pipeName.c_str(),
                                    PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
                                    PIPE_TYPE_BYTE | PIPE_WAIT, 1, bufSize, 0, 0, nullptr);

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Pipe creation error: %lu", GetLastError());
        return 0;
    }

    OVERLAPPED ov;
    ZeroMemory(&ov, sizeof(ov));
    ov.hEvent = CreateEventA(nullptr, false, false, nullptr);

    while (true)
    {
        int iResult = ConnectNamedPipe(hPipe, &ov);
        if (iResult != 0)
        {
            fprintf(stderr, "Pipe connection failed: %lu", GetLastError());
            continue;
        }

        iResult = WaitForSingleObjectEx(ov.hEvent, INFINITE, true);
        if (iResult == WAIT_IO_COMPLETION && stopPipeThread)
        {
            break;
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
    CloseHandle(ov.hEvent);

    printf("Pipe thread closed\n");

    return 0;
}
