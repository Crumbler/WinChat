#include "ServerConfig.hpp"

void ServerConfig::setPipeName(const char* s)
{
    this->pipeName = "\\\\.\\pipe\\";
    this->pipeName += s;
}
