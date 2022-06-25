#pragma once

class ServerConfig
{
public:
    ServerConfig();
    ~ServerConfig();

    int port, threadCount, listenQueueSize,
        spinCount;
};
