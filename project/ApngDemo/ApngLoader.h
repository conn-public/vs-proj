#pragma once
#include "png.h"

#include <gdiplus.h>
#include <vector>
#include <string>
#include <memory>
using namespace Gdiplus;
using namespace std;

struct ApngFrame
{
    UINT32 nFrameID = 0;            //id
    std::vector<BYTE> vecFrameBits; //rgb
    UINT32 nDelayMS = 0;            //
};

typedef std::shared_ptr<ApngFrame> ApngFramePtr;
typedef std::vector<ApngFramePtr>  ApngFrameVec;

struct ApngImage
{
    UINT32 nWidth = 0;
    UINT32 nHeight = 0;
    ApngFrameVec vecFrames;
};

class CApngLoader
{
public:
    CApngLoader();
    ~CApngLoader();

    BOOL LoadAPNG(const wstring& strImg);

    Bitmap* GetFrameAt(UINT32 nIndex, UINT32& nDelayMS);

    UINT32 GetFrameCount() { return m_apng.vecFrames.size(); }

    void CleanUp();
protected:
    BOOL MakeFrame(BYTE** rows, UINT32 w, UINT32 h, UINT32 channels, UINT32 frame, UINT32 nDelayMS);

    void BlendOver(BYTE** rows_dst, BYTE** rows_src, UINT32 x, UINT32 y, UINT32 w, UINT32 h);

protected:
    ApngImage m_apng;
    UINT32 m_nRowSize = 0;      //一行像素大小 BYTE
    UINT32 m_nFrameSize = 0;    //一帧图片像素大小 BYTE
    bool m_bFirstFrameIsHidden = false; //APNG默认图片是不是包含在动画里面
    UINT32 m_nWidth = 0;        //宽
    UINT32 m_nHeight = 0;       //高
    UINT32 m_nChannels = 0;     //1,3,4

    Bitmap* m_pBmp = NULL;

    UINT32 m_nPlays = 0;        //播放几次
    UINT32 m_nCurFrame = 0;     //当前帧ID
    //UINT32 m_nCurPlays = 0;     //当前播放次数
};

