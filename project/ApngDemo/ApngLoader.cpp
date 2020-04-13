#include "stdafx.h"
#include "ApngLoader.h"


CApngLoader::CApngLoader()
{
}


CApngLoader::~CApngLoader()
{
}

BOOL CApngLoader::LoadAPNG(const wstring& strImg)
{
    FILE* f1 = NULL;
    _wfopen_s(&f1, strImg.c_str(), L"rb");
    if (f1 == NULL)
    {
        OutputDebugString(L"Cannot open file!\r\n");
        return FALSE;
    }

    UINT32 i, j;
    png_bytepp rows_image;
    png_bytepp rows_frame;
    BYTE* p_image;
    BYTE* p_frame;
    BYTE* p_temp;
    BYTE sig[8];

    fread(sig, 1, 8, f1);
    if (png_sig_cmp(sig, 0, 8) != 0)
    {
        OutputDebugString(L"Not a png image!\r\n");
        fclose(f1);
        return FALSE;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (png_ptr && info_ptr)
    {
        if (setjmp(png_jmpbuf(png_ptr)))
        {
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(f1);
            return FALSE;
        }
        png_init_io(png_ptr, f1);
        png_set_sig_bytes(png_ptr, 8);
        png_read_info(png_ptr, info_ptr);

        if (!png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL))
        {
            OutputDebugString(L"Not a apng image!\r\n");
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(f1);
            return FALSE;
        }

        UINT32 nFrameCount = 1;
        m_nPlays = 0;
        png_get_acTL(png_ptr, info_ptr, &nFrameCount, &m_nPlays);
        if (nFrameCount <= 1)
        {
            OutputDebugString(L"Not a apng Image!\r\n");
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(f1);
            return FALSE;
        }

        png_set_expand(png_ptr);
        png_set_strip_16(png_ptr);
        png_set_gray_to_rgb(png_ptr);
        png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
        png_set_bgr(png_ptr);
        png_set_interlace_handling(png_ptr);
        png_read_update_info(png_ptr, info_ptr);

        m_nWidth = png_get_image_width(png_ptr, info_ptr);
        m_nHeight = png_get_image_height(png_ptr, info_ptr);
        m_nChannels = png_get_channels(png_ptr, info_ptr);
        m_nRowSize = png_get_rowbytes(png_ptr, info_ptr);
        m_nFrameSize = m_nHeight * m_nRowSize;
        p_image = (BYTE*)malloc(m_nFrameSize);
        p_frame = (BYTE*)malloc(m_nFrameSize);
        p_temp = (BYTE*)malloc(m_nFrameSize);
        rows_image = (png_bytepp)malloc(m_nHeight * sizeof(png_bytep));
        rows_frame = (png_bytepp)malloc(m_nHeight * sizeof(png_bytep));
        m_bFirstFrameIsHidden = png_get_first_frame_is_hidden(png_ptr, info_ptr) != 0;
        if (p_image && p_frame && p_temp && rows_image && rows_frame)
        {
            png_uint_32 x0 = 0;
            png_uint_32 y0 = 0;
            png_uint_32 w0 = m_nWidth;
            png_uint_32 h0 = m_nHeight;
            unsigned short delay_num = 1;
            unsigned short delay_den = 10;
            BYTE dop = 0;
            BYTE bop = 0;
            UINT32 first = (m_bFirstFrameIsHidden != 0) ? 1 : 0;

            for (j = 0; j < m_nHeight; j++)
                rows_image[j] = p_image + j * m_nRowSize;

            for (j = 0; j < m_nHeight; j++)
                rows_frame[j] = p_frame + j * m_nRowSize;

            for (i = 0; i < nFrameCount; i++)
            {
                png_read_frame_head(png_ptr, info_ptr);
                png_get_next_frame_fcTL(png_ptr, info_ptr, &w0, &h0, &x0, &y0, &delay_num, &delay_den, &dop, &bop);

                if (i == first)
                {
                    bop = PNG_BLEND_OP_SOURCE;
                    if (dop == PNG_DISPOSE_OP_PREVIOUS)
                        dop = PNG_DISPOSE_OP_BACKGROUND;
                }

                png_read_image(png_ptr, rows_frame);

                if (dop == PNG_DISPOSE_OP_PREVIOUS)
                    memcpy(p_temp, p_image, m_nFrameSize);

                if (bop == PNG_BLEND_OP_OVER)
                {
                    BlendOver(rows_image, rows_frame, x0, y0, w0, h0);
                }
                else
                {
                    for (j = 0; j < h0; j++)
                        memcpy(rows_image[j + y0] + x0 * 4, rows_frame[j], w0 * 4);
                }


                MakeFrame(rows_image, m_nWidth, m_nHeight, m_nChannels, i, delay_num * 1000 / delay_den);

                if (dop == PNG_DISPOSE_OP_PREVIOUS)
                {
                    memcpy(p_image, p_temp, m_nFrameSize);
                }
                else if (dop == PNG_DISPOSE_OP_BACKGROUND)
                {
                    for (j = 0; j < h0; j++)
                        memset(rows_image[j + y0] + x0 * 4, 0, w0 * 4);
                }
            }

            png_read_end(png_ptr, info_ptr);
            free(rows_frame);
            free(rows_image);
            free(p_temp);
            free(p_frame);
            free(p_image);
        }
    }
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    fclose(f1);
    return TRUE;
}

Bitmap* CApngLoader::GetFrameAt(UINT32 nIndex, UINT32& nDelayMS)
{
    if (nIndex >= m_apng.vecFrames.size())
        return m_pBmp;

    ApngFramePtr ptrFrame = m_apng.vecFrames[nIndex];
    if (!ptrFrame || ptrFrame->vecFrameBits.size() < m_nFrameSize)
        return m_pBmp;

    LPBYTE pBuf = &ptrFrame->vecFrameBits[0];
    //SetFilePointer(m_hTempFile, dwLow, &dwHigh, SEEK_SET);


    Bitmap* pBmp = m_pBmp ? m_pBmp : new Bitmap(m_nWidth, m_nHeight);
    Rect rc(0, 0, m_nWidth, m_nHeight);
    BitmapData bmd;
    pBmp->LockBits(&rc, ImageLockModeWrite, PixelFormat32bppARGB, &bmd);
    LPBYTE pDst = (LPBYTE)bmd.Scan0;
    for (UINT32 i = 0; i < m_nHeight; ++i)
    {
        //ReadFile(m_hTempFile, pDst, rowsize, &ret, NULL);
        memcpy(pDst, pBuf, m_nRowSize);
        pDst += bmd.Stride;
        pBuf += m_nRowSize;
    }
    pBmp->UnlockBits(&bmd);

    m_nCurFrame = nIndex;
    m_pBmp = pBmp;
    nDelayMS = ptrFrame->nDelayMS;
    return pBmp;
}

void CApngLoader::CleanUp()
{
    if (m_pBmp)
    {
        delete m_pBmp;
        m_pBmp = NULL;
    }
    
    m_apng.vecFrames.clear();
}

BOOL CApngLoader::MakeFrame(BYTE** rows, UINT32 w, UINT32 h, UINT32 channels, UINT32 frame, UINT32 nDelayMS)
{
    ApngFramePtr ptrFrame(new ApngFrame);
    if (!ptrFrame)
        return FALSE;

    ptrFrame->nFrameID = frame;
    ptrFrame->nDelayMS = nDelayMS;
    ptrFrame->vecFrameBits.resize(h * m_nRowSize);

    LPBYTE pBuf = &ptrFrame->vecFrameBits[0];
    for (UINT32 i = 0; i < h; ++i)
    {
        memcpy(pBuf, rows[i], m_nRowSize);
        pBuf += m_nRowSize;
    }

    m_apng.vecFrames.push_back(ptrFrame);
    return TRUE;
}

void CApngLoader::BlendOver(BYTE** rows_dst, BYTE** rows_src, UINT32 x, UINT32 y, UINT32 w, UINT32 h)
{
    UINT32 i, j;
    int u, v, al;

    for (j = 0; j < h; j++)
    {
        BYTE* sp = rows_src[j];
        BYTE* dp = rows_dst[j + y] + x * 4;

        for (i = 0; i < w; i++, sp += 4, dp += 4)
        {
            if (sp[3] == 255)
                memcpy(dp, sp, 4);
            else if (sp[3] != 0)
            {
                if (dp[3] != 0)
                {
                    u = sp[3] * 255;
                    v = (255 - sp[3]) * dp[3];
                    al = u + v;
                    dp[0] = (sp[0] * u + dp[0] * v) / al;
                    dp[1] = (sp[1] * u + dp[1] * v) / al;
                    dp[2] = (sp[2] * u + dp[2] * v) / al;
                    dp[3] = al / 255;
                }
                else
                    memcpy(dp, sp, 4);
            }
        }
    }
}
