#include "stdafx.h"
#include "GifLoader.h"


CGifLoader::CGifLoader()
{
}


CGifLoader::~CGifLoader()
{
}

BOOL CGifLoader::LoadImage(const wstring& strImg)
{
    Bitmap* pBmp = Bitmap::FromFile(strImg.c_str());
    if (!pBmp || pBmp->GetLastStatus() != Ok || GetImageMimeType(pBmp) != L"image/gif")
    {
        if (pBmp) delete pBmp;
        return FALSE;
    }

    UINT nCount = pBmp->GetFrameDimensionsCount();
    GUID* pDimensionIDs = new GUID[nCount];
    pBmp->GetFrameDimensionsList(pDimensionIDs, nCount);

    m_nFrameCount = pBmp->GetFrameCount(&pDimensionIDs[0]);
    if (m_nFrameCount > 1)
    {        int nSize = pBmp->GetPropertyItemSize(PropertyTagFrameDelay);        if (nSize)        {            m_pPropertyDelay = (Gdiplus::PropertyItem*)new BYTE[nSize];            if (Gdiplus::Ok != pBmp->GetPropertyItem(PropertyTagFrameDelay, nSize, m_pPropertyDelay))            {                free(m_pPropertyDelay);                m_pPropertyDelay = NULL;            }        }
        nSize = pBmp->GetPropertyItemSize(PropertyTagLoopCount);
        if (nSize)
        {
            Gdiplus::PropertyItem* pLoops = (Gdiplus::PropertyItem*)new BYTE[nSize];
            if (Gdiplus::Ok == pBmp->GetPropertyItem(PropertyTagLoopCount, nSize, pLoops))
                m_nPlays = ((LONG*)pLoops->value)[0];

            delete[] pLoops;
        }
    }

    delete[]  pDimensionIDs;
    m_pBmp = pBmp;
    return TRUE;
}

Bitmap* CGifLoader::GetFrameAt(UINT32 nIndex, UINT32& nDelayMS)
{
    if (!m_pBmp)
        return NULL;

    if (m_nFrameCount > 1 && nIndex < m_nFrameCount)
    {
        GUID pageGuid = Gdiplus::FrameDimensionTime;
        m_pBmp->SelectActiveFrame(&pageGuid, nIndex);
        if (m_pPropertyDelay)
            nDelayMS = ((LONG*)m_pPropertyDelay->value)[nIndex] * 10;
        else
            nDelayMS = -1;
    }
    return m_pBmp;
}

void CGifLoader::CleanUp()
{
    if (m_pBmp)
    {
        delete m_pBmp;
        m_pBmp = NULL;
    }

    if (m_pPropertyDelay)
    {
        delete[] m_pPropertyDelay;
        m_pPropertyDelay = NULL;
    }
}
