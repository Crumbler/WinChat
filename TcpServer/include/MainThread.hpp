#pragma once

#include <windows.h>

unsigned _stdcall MainThread(void *param);
void StopMainThread(ULONG_PTR p);
