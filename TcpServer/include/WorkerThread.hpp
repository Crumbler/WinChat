#pragma once

#include <string>
#include <unordered_map>
#include "ClientKey.hpp"

unsigned __stdcall WorkerThread(void *param);

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

extern std::unordered_map<std::string*, ClientKey*, strPtrHash, strPtrEqual> clients;
