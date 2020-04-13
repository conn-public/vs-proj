#include "stdafx.h"
#include "WebPLoader.h"
#include "StringUtil.hpp"


CWebPLoader::CWebPLoader()
{
}


CWebPLoader::~CWebPLoader()
{
}

BOOL CWebPLoader::LoadImage(const wstring& strImg)
{
    return ReadAnimatedImage(CStringUtil::ToStringA(strImg).c_str(), &image);
}

Bitmap* CWebPLoader::GetFrameAt(UINT32 nIndex, UINT32& nDelayMS)
{
    if (nIndex >= image.num_frames)
        return m_pBmp;

    DecodedFrame* pFrame = &image.frames[nIndex];
    if (!pFrame->rgba)
        return m_pBmp;

    Bitmap* pBmp = m_pBmp ? m_pBmp : new Bitmap(image.canvas_width, image.canvas_height);
    BitmapData bmd;
    Rect rc(0, 0, image.canvas_width, image.canvas_height);
    pBmp->LockBits(&rc, ImageLockModeRead | ImageLockModeWrite, PixelFormat32bppARGB, &bmd);
    LPBYTE pDst = (LPBYTE)bmd.Scan0;
    LPBYTE pSrc = pFrame->rgba;

    int rowsize = rc.Width * 4;
    for (int h = 0; h < rc.Height; ++h)
    {
        LPBYTE pDstLine = pDst;
        for (int w=0; w<rc.Width; ++w)
        {
            //RGB->BGR
            pDstLine[0] = pSrc[2];
            pDstLine[1] = pSrc[1];
            pDstLine[2] = pSrc[0];
            pDstLine[3] = pSrc[3];
            pSrc += 4;
            pDstLine += 4;
        }
        //memcpy(pDst, pSrc, rowsize);
        //pSrc += rowsize;
        pDst += bmd.Stride;
    }
    pBmp->UnlockBits(&bmd);
    m_nCurFrame = nIndex;
    m_pBmp = pBmp;
    nDelayMS = pFrame->duration;
    return pBmp;
}

void CWebPLoader::CleanUp()
{
    if (m_pBmp)
    {
        delete m_pBmp;
        m_pBmp = NULL;
    }

    ClearAnimatedImage(&image);
}
