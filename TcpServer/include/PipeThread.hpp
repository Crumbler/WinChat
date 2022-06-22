#pragma once

#include <windows.h>

unsigned __stdcall PipeThread(void *param);
void StopPipeThread(ULONG_PTR p);
