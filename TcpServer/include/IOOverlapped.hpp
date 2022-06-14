#pragma once

#include <winsock2.h>
#include "IOType.hpp"

struct IOOverlapped
{
    OVERLAPPED ov;
    IOType type;
};
