#pragma once

#include <windows.h>
#include <string>
#include <vector>

constexpr wchar_t szLogControl[] = L"LogControl";

constexpr int WM_LOGADD = WM_USER + 1,
              WM_LOGCLEAR = WM_LOGADD + 1;

void RegisterLogControl(HINSTANCE hInstance);

struct LogControlExtra
{
    int width, height;
    std::vector<std::string> lines;
};
