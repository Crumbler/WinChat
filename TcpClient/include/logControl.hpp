#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <gdiplus.h>

constexpr wchar_t szLogControl[] = L"LogControl";

constexpr int WM_LOGADD = WM_USER + 1,
              WM_LOGCLEAR = WM_LOGADD + 1;

void RegisterLogControl(HINSTANCE hInstance);

struct LogControlExtra
{
    int width, height, textHeight, scrollPos;
    Gdiplus::Font* fontMain;
    Gdiplus::StringFormat* strfrmMain;
    std::vector<std::wstring> lines;
    std::vector<float> lineHeights;
};
