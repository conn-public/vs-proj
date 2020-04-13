#pragma once
#include <windef.h>
#include <string>
#include <vector>

#define CP_CHS      936
#define LCID_CHS    0x0804

class CStringUtil
{
protected:
    static UINT CheckCodePage(UINT uCodePage)
    {
        if (uCodePage == CP_ACP)
        {
            LCID lcid = GetSystemDefaultLCID();
            if (lcid != LCID_CHS)  //系统区域不是中华人民共和国 
            {
                return CP_CHS;
            }
        }

        return uCodePage;
    }

    static BOOL MyW2A(LPCWSTR wstr, std::string & str, UINT uCodePage)
    {
        if (!wstr) return FALSE;

        uCodePage = CheckCodePage(uCodePage);

        int nRet = WideCharToMultiByte(uCodePage, 0, wstr, -1, 0, 0, 0, 0);
        if (nRet)
        {
            std::vector<CHAR> buf(nRet + 1);
            CHAR * pStr = &buf[0];

            nRet = WideCharToMultiByte(uCodePage, 0, wstr, -1, pStr, nRet + 1, 0, 0);
            if (nRet)
            {
                pStr[nRet] = '\0';
                str = pStr;
                return TRUE;
            }

        }

        return FALSE;
    }


    static BOOL MyA2W(LPCSTR str, std::wstring & wstr, UINT uCodePage)
    {
        uCodePage = CheckCodePage(uCodePage);

        int nRet = MultiByteToWideChar(uCodePage, 0, str, -1, 0, 0);

        if (nRet)
        {
            std::vector<WCHAR> buf(nRet + 1);
            WCHAR * pWstr = &buf[0];

            nRet = MultiByteToWideChar(uCodePage, 0, str, -1, pWstr, nRet + 1);
            if (nRet)
            {
                pWstr[nRet] = L'\0';
                wstr = pWstr;
                return TRUE;
            }
        }

        return FALSE;
    }


public:
    static std::string ToStringA(const std::wstring& str, const UINT uCodePage = CP_ACP)
    {
        std::string strRet;
        MyW2A(str.c_str(), strRet, uCodePage);
        return strRet;
    }

    static std::wstring ToStringW(const std::string& str, const UINT uCodePage = CP_ACP)
    {
        std::wstring strRet;
        MyA2W(str.c_str(), strRet, uCodePage);
        return strRet;
    }

};
