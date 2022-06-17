
#include "logControl.hpp"
#include "logControlFuncs.hpp"

#include <gdiplus.h>

using namespace Gdiplus;

constexpr int pageUnits = 20;

LogControlExtra* GetLogControl(HWND hwnd)
{
    return (LogControlExtra*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}

void CalcTextHeight(HWND hwnd)
{
    LogControlExtra *logCtrl = GetLogControl(hwnd);

    HDC hdc = GetDC(hwnd);

    Graphics graphics(hdc);

    float totalHeight = 0.0f;

    const RectF rectLayout(0.0f, 0.0f, logCtrl->width, 1.e7f);

    for (size_t i = 0; i < logCtrl->lines.size(); ++i)
    {
        const std::wstring& s = logCtrl->lines[i];
        RectF rectBounds;
        graphics.MeasureString(s.data(), -1, logCtrl->fontMain, rectLayout, &rectBounds);

        logCtrl->lineHeights[i] = rectBounds.Height;
        totalHeight += rectBounds.Height;
    }

    logCtrl->textHeight = totalHeight;

    ReleaseDC(hwnd, hdc);
}

void CalcScrollInfo(HWND hwnd)
{
    LogControlExtra *logCtrl = GetLogControl(hwnd);

    SCROLLINFO si;

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = logCtrl->textHeight / pageUnits;
    si.nPage = logCtrl->height / pageUnits;

    SetScrollInfo(hwnd, SB_VERT, &si, true);

    si.fMask = SIF_POS;
    GetScrollInfo(hwnd, SB_VERT, &si);
    logCtrl->scrollPos = si.nPos;
}

void ScrollToBottom(HWND hwnd)
{
    LogControlExtra *logCtrl = GetLogControl(hwnd);

    SCROLLINFO si;

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE;

    GetScrollInfo(hwnd, SB_VERT, &si);

    si.fMask = SIF_POS;
    logCtrl->scrollPos = si.nMax;
    si.nPos = si.nMax;
    SetScrollInfo(hwnd, SB_VERT, &si, false);
}

void OnLogCreate(HWND hwnd)
{
    auto *logCtrl = new LogControlExtra();

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)logCtrl);

    logCtrl->fontMain = new Font(L"Times New Roman", 14);
    logCtrl->strfrmMain = StringFormat::GenericDefault()->Clone();
}

void OnLogClose(HWND hwnd)
{
    auto *logCtrl = GetLogControl(hwnd);

    delete logCtrl;
}

void OnLogPaint(HWND hwnd)
{
    LogControlExtra *logCtrl = GetLogControl(hwnd);

    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    Graphics graphics(hdc);

    graphics.TranslateTransform(0.0f, -logCtrl->scrollPos * pageUnits);

    SolidBrush brushBlack(Color::Black);

    RectF rectLayout;

    rectLayout.X = 0.0f;
    rectLayout.Width = logCtrl->width;

    for (size_t i = 0; i < logCtrl->lines.size(); ++i)
    {
        const std::wstring& s = logCtrl->lines[i];
        rectLayout.Height = logCtrl->lineHeights[i];
        graphics.DrawString(s.data(), -1, logCtrl->fontMain, rectLayout,
                            logCtrl->strfrmMain, &brushBlack);

        rectLayout.Y += rectLayout.Height;
    }

    EndPaint(hwnd, &ps);
}

void OnLogVScroll(HWND hwnd, WPARAM wParam)
{
    LogControlExtra *logCtrl = GetLogControl(hwnd);

    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;

    GetScrollInfo(hwnd, SB_VERT, &si);

    int iVertPos = si.nPos;

    switch (LOWORD(wParam))
    {
    case SB_TOP:
        si.nPos = si.nMin;
        break;

    case SB_BOTTOM:
        si.nPos = si.nMax;
        break;

    case SB_LINEUP:
        --si.nPos;
        break;

    case SB_LINEDOWN:
        ++si.nPos;
        break;

    case SB_PAGEUP:
        si.nPos -= si.nPage;
        break;

    case SB_PAGEDOWN:
        si.nPos += si.nPage;
        break;

    case SB_THUMBPOSITION:
        si.nPos = si.nTrackPos;
        break;
    }

    si.fMask = SIF_POS;
    SetScrollInfo(hwnd, SB_VERT, &si, false);
    GetScrollInfo(hwnd, SB_VERT, &si);

    if (si.nPos != iVertPos)
    {
        logCtrl->scrollPos = si.nPos;
        InvalidateRect(hwnd, nullptr, true);
    }
}

void OnLogResize(HWND hwnd, int newWidth, int newHeight)
{
    auto *logCtrl = GetLogControl(hwnd);

    logCtrl->width = newWidth;
    logCtrl->height = newHeight;

    CalcTextHeight(hwnd);
    CalcScrollInfo(hwnd);
}

void OnLogAdd(HWND hwnd, const char *m)
{
    LogControlExtra *logCtrl = GetLogControl(hwnd);

    const int len = strlen(m);

    std::wstring s(&m[0], &m[len]);

    logCtrl->lines.emplace_back(s);
    logCtrl->lineHeights.push_back(0.0f);

    CalcTextHeight(hwnd);
    CalcScrollInfo(hwnd);

    ScrollToBottom(hwnd);

    InvalidateRect(hwnd, nullptr, true);
}

void OnLogClear(HWND hwnd)
{
    LogControlExtra *logCtrl = GetLogControl(hwnd);

    logCtrl->lines.clear();
    logCtrl->lineHeights.clear();

    CalcTextHeight(hwnd);
    CalcScrollInfo(hwnd);

    InvalidateRect(hwnd, nullptr, true);
}
