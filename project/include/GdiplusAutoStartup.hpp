#pragma once
#include <gdiplus.h>
using namespace Gdiplus;

class CGdiplusAutoStartup
{
public:
    CGdiplusAutoStartup::CGdiplusAutoStartup()
    {
        Status Ret = Ok;
        GdiplusStartupInput gsi;

        Ret = GdiplusStartup(&m_uToken, &gsi, NULL);

        if (Ok != Ret)
        {
            OutputDebugStringA("Cannot init gdi+ \n");
        }
    }

    CGdiplusAutoStartup::~CGdiplusAutoStartup()
    {
        if (0 != m_uToken)
        {
            GdiplusShutdown(m_uToken);
        }
    }

public:

    ULONG_PTR m_uToken;
};

#ifndef GDIPLUS_AUTO_STARTUP
#   define GDIPLUS_AUTO_STARTUP static CGdiplusAutoStartup __s_gidp_starup__
#endif

