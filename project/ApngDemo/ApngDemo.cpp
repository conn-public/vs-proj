// apng-demo.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ApngDemo.h"
#include "GdiplusAutoStartup.hpp"
#include "ApngLoader.h"

#define MAX_LOADSTRING 100

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

CApngLoader g_apngLoader;
UINT32      g_nCurrentFrameID = 0;

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此放置代码。
    GDIPLUS_AUTO_STARTUP;
    WCHAR szPath[MAX_PATH + 1];
    GetModuleFileName(NULL, szPath, _countof(szPath));
    PathRemoveFileSpec(szPath);
    PathAppend(szPath, L"\\..\\..\\..\\test\\clock.png");
    if (!g_apngLoader.LoadAPNG(szPath))
    {
        MessageBox(NULL, L"Cannot load image", L"Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_APNGDEMO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_APNGDEMO));

    MSG msg;

    // 主消息循环: 
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APNGDEMO));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_APNGDEMO);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        //first frame
        SetTimer(hWnd, 1234, 500, NULL);
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择: 
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...

            RECT rcWin;
            GetClientRect(hWnd, &rcWin);
            Rect rcBg(0, 0, rcWin.right - rcWin.left, rcWin.bottom - rcWin.top);
            
            HDC hMemDC = ::CreateCompatibleDC(hdc);
            HBITMAP hBmpOffscreen = ::CreateCompatibleBitmap(hdc, rcBg.Width, rcBg.Height);
            HBITMAP hBmpOld = (HBITMAP)::SelectObject(hMemDC, hBmpOffscreen);


            Graphics gr(hMemDC);
            gr.Clear(0xFFFFFFFFu);

            UINT32 nDelayMS = 0;
            Bitmap* pBmp = g_apngLoader.GetFrameAt(g_nCurrentFrameID, nDelayMS);
            if (pBmp)
            {

                Rect rc(0, 0, rcWin.right - rcWin.left, rcWin.bottom - rcWin.top);
                rc.Offset((rc.Width - (INT)pBmp->GetWidth()) / 2, (rc.Height - (INT)pBmp->GetHeight()) / 2);
                rc.Width = pBmp->GetWidth();
                rc.Height = pBmp->GetHeight();
                gr.DrawImage(pBmp, rc);
            }

            BitBlt(hdc, 0, 0, rcBg.Width, rcBg.Height, hMemDC, 0, 0, SRCCOPY);
            ::SelectObject(hMemDC, hBmpOld);
            DeleteObject(hBmpOffscreen);
            DeleteDC(hMemDC);

            EndPaint(hWnd, &ps);

            //next
            SetTimer(hWnd, 1234, nDelayMS, NULL);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_TIMER:
        if (wParam == 1234)
        {
            g_nCurrentFrameID++;
            g_nCurrentFrameID %= g_apngLoader.GetFrameCount();
            RECT rc;
            GetClientRect(hWnd, &rc);
            InvalidateRect(hWnd, &rc, TRUE);
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
