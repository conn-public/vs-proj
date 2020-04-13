#pragma once
#include <string>
using namespace std;

class CGifLoader
{
public:
    CGifLoader();
    ~CGifLoader();

    BOOL LoadImage(const wstring& strImg);

    Bitmap* GetFrameAt(UINT32 nIndex, UINT32& nDelayMS);

    UINT32 GetFrameCount() { return m_nFrameCount; }

    void CleanUp();

protected:
    std::wstring GetImageMimeType(Gdiplus::Bitmap* pBmp)
    {
        std::wstring strRes = L"image/unknown";
        if (!pBmp) return strRes;

        UINT  num = 0;          // number of image encoders
        UINT  size = 0;         // size of the image encoder array in bytes

        Gdiplus::GetImageDecodersSize(&num, &size);
        if (size == 0) return strRes;

        Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
        if (pImageCodecInfo == NULL) return strRes;

        GUID guid;
        pBmp->GetRawFormat(&guid);

        Gdiplus::GetImageDecoders(num, size, pImageCodecInfo);
        for (UINT j = 0; j < num; ++j)
        {
            if (IsEqualGUID(pImageCodecInfo[j].FormatID, guid))
            {
                strRes = pImageCodecInfo[j].MimeType;
                break;
            }
        }

        free(pImageCodecInfo);
        return strRes;
    }

protected:
    Bitmap* m_pBmp = NULL;
    PropertyItem* m_pPropertyDelay = NULL;

    UINT32 m_nFrameCount = 0;
    UINT32 m_nPlays = 0;        //播放几次
    UINT32 m_nCurFrame = 0;     //当前帧ID
                                //UINT32 m_nCurPlays = 0;     //当前播放次数
};

