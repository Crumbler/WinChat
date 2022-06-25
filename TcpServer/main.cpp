
#include <cstdio>
#include <stdexcept>
#include "TcpSocket.hpp"
#include "MainThread.hpp"
#include "ServerConfig.hpp"
#include "pugixml.hpp"
#include <windows.h>

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE statusHandle = nullptr;

HANDLE hMainThread;

void WINAPI ServiceMain(DWORD argc, char *argv);
void WINAPI ServiceCtrlHandler(DWORD);

WSADATA wsaData;

ServerConfig *config;
char serviceName[] = "Chat server";

bool Startup();
void Cleanup();

int main()
{
    SERVICE_TABLE_ENTRYA ServiceTable[] =
    {
        { serviceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { nullptr, nullptr }
    };

    if (!StartServiceCtrlDispatcher(ServiceTable))
    {
        return GetLastError();
    }

    return 0;
}

void WINAPI ServiceMain(DWORD argc, char *argv)
{
    statusHandle = RegisterServiceCtrlHandlerA(serviceName, ServiceCtrlHandler);

    if (statusHandle == nullptr)
    {
        return;
    }

    ZeroMemory(&serviceStatus, sizeof(serviceStatus));

    serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    SetServiceStatus(statusHandle, &serviceStatus);

    if (!Startup())
    {
        return;
    }

    serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(statusHandle, &serviceStatus);

    hMainThread = (HANDLE)_beginthreadex(nullptr, 0, MainThread, config, 0, nullptr);

    WaitForSingleObject(hMainThread, INFINITE);
    CloseHandle(hMainThread);

    Cleanup();

    serviceStatus.dwControlsAccepted = 0;
    serviceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus (statusHandle, &serviceStatus);
}

bool Startup()
{
    constexpr int defaultPort = 27000,
        defaultListenQueueSize = 10,
        defaultSpinCount = 4000;

    constexpr char defaultPipeName[] = "tcpwatcher";

    constexpr WORD wVersionRequested = MAKEWORD(2, 2);

    setbuf(stderr, nullptr);
    setbuf(stdout, nullptr);

    int iResult = WSAStartup(wVersionRequested, &wsaData);
    if (iResult != 0)
    {
        serviceStatus.dwCurrentState = SERVICE_STOPPED;
        serviceStatus.dwWin32ExitCode = iResult;
        SetServiceStatus(statusHandle, &serviceStatus);
        return false;
    }

    config = new ServerConfig();

    wchar_t *path = new wchar_t[256];
    GetModuleFileNameW(nullptr, path, 256);

    wchar_t *s = wcsrchr(path, L'\\');

    // Replace executable name with the config file name
    wcscpy(s + 1, L"config.xml");

    pugi::xml_document doc;

    doc.load_file(path);

    pugi::xml_node root = doc.child("ServerConfig");

    config->port = root.child("Port").text().as_int(defaultPort);
    config->threadCount = root.child("ThreadCount").text().as_int(0);
    config->listenQueueSize = root.child("ListenQueueSize").text().as_int(defaultListenQueueSize);
    config->spinCount = root.child("SpinCount").text().as_int(defaultSpinCount);

    const char *pipeName = root.child("PipeName").text().as_string(defaultPipeName);
    config->setPipeName(pipeName);

    delete[] path;

    return true;
}

void Cleanup()
{
    delete config;

    if (WSACleanup() != 0)
    {
        serviceStatus.dwCurrentState = SERVICE_STOPPED;
        serviceStatus.dwWin32ExitCode = WSAGetLastError();
        SetServiceStatus(statusHandle, &serviceStatus);
    }
}

void WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
    switch (CtrlCode)
    {
    case SERVICE_CONTROL_STOP:
        if (serviceStatus.dwCurrentState != SERVICE_RUNNING)
        {
            break;
        }

        serviceStatus.dwControlsAccepted = 0;
        serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(statusHandle, &serviceStatus);

        QueueUserAPC(StopMainThread, hMainThread, (ULONG_PTR)nullptr);

        break;

    case SERVICE_CONTROL_INTERROGATE:
        break;
    }
}
