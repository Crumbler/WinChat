#pragma once

#include <string>

struct ServerConfig
{
    int port, threadCount, listenQueueSize,
        spinCount;

    std::string pipeName;

    void setPipeName(const char *s);
};
