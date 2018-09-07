// HelloWindowsDesktop.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#define _WINSOCKAPI_
#define IDT_TIMER1 1001

#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <vector>
#include <sstream>
// GDI+
#include <gdiplus.h>
#include <objidl.h>
// Curl
#include <curl.h>
#include <atlstr.h>

#pragma comment(lib,"gdiplus.lib")

using namespace Gdiplus;

// Global variables

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Room Monitor Ver1.0");

HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::vector<std::string> separate_by_comma(std::string input) {
    std::stringstream ss(input);
    std::vector<std::string> result;

    while (ss.good())
    {
        std::string substr;
        getline(ss, substr, ',');
        result.push_back(substr);
    }

    return result;
}

int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
){
    WNDCLASSEX wcex;
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    // Store instance handle in our global variable
    hInst = hInstance;

    // The parameters to CreateWindow explained:
    // szWindowClass: the name of the application
    // szTitle: the text that appears in the title bar
    // WS_OVERLAPPEDWINDOW: the type of window to create
    // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
    // 500, 300: initial size (width, length)
    // NULL: the parent of this window
    // NULL: this application does not have a menu bar
    // hInstance: the first parameter from WinMain
    // NULL: not used in this application
    HWND hWnd = CreateWindow(
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        500, 300,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    HWND hwndButton = CreateWindow(
        L"BUTTON",      // Predefined class; Unicode assumed 
        L"INFO",        // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        190,            // x position 
        200,            // y position 
        90,             // Button width
        20,             // Button height
        hWnd,           // Parent window
        (HMENU)1,       // Menu 1
        (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
        NULL);         // Pointer not needed.

    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    // The parameters to ShowWindow explained:
    // hWnd: the value returned from CreateWindow
    // nCmdShow: the fourth parameter from WinMain
    ShowWindow(hWnd,
        nCmdShow);
    UpdateWindow(hWnd);

    SetTimer(hWnd,              // handle to main window 
        IDT_TIMER1,             // timer identifier 
        60000,                  // 60-second interval 
        (TIMERPROC)NULL);       // no timer callback 

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    HFONT hFont1, hFont2, hTmp;
    static TCHAR greeting[] = _T("Monitor The Room");
    static TCHAR infos[] = _T("Top Contributors: Mingze, Mars\nSpecial Thanks:     NB KYC");
    static bool kycnb = false;
    static std::string url = "http://demo.marstanjx.com/weather/current.php";
    static Bitmap* image_temp = Bitmap::FromFile(L"temp.png");
    static Bitmap* image_hum = Bitmap::FromFile(L"humidity.png");
    TCHAR time_char[32];
    TCHAR temp_char[32];
    TCHAR hum_char[32];
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    switch (message)
    {
    case WM_PAINT:
    {
        hdc = BeginPaint(hWnd, &ps);
        hFont1 = CreateFont(25, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, L"SYSTEM_FIXED_FONT");
        hTmp = (HFONT)SelectObject(hdc, hFont1);
        SetBkMode(hdc, TRANSPARENT);
        TextOut(hdc,
            150, 5,
            greeting, _tcslen(greeting));

        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            else {
                std::vector<std::string> separated = separate_by_comma(readBuffer);
                _tcscpy_s(time_char, CA2T(separated[0].substr(0, separated[0].size()-3).c_str()));
                _tcscpy_s(temp_char, CA2T(separated[1].c_str()));
                _tcscpy_s(hum_char, CA2T(separated[2].c_str()));
                TextOut(hdc,
                    150, 230,
                    time_char, _tcslen(time_char));

                hFont2 = CreateFont(50, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, L"SYSTEM_FIXED_FONT");
                hTmp = (HFONT)SelectObject(hdc, hFont2);

                TextOut(hdc,
                    75, 50,
                    temp_char, _tcslen(temp_char));
                TextOut(hdc,
                    330, 50,
                    hum_char, _tcslen(hum_char));
            }
            curl_easy_cleanup(curl);
        }

        Graphics graphics(hdc);
        // Display image
        graphics.DrawImage(image_temp, 50, 100);
        graphics.DrawImage(image_hum, 300, 100);

        DeleteObject(SelectObject(hdc, hTmp));
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 1:
            ::MessageBeep(MB_ICONERROR);
            ::MessageBox(hWnd, infos, L"Distribution Info", MB_OK);
            break;
        }
        break;
    case WM_TIMER:
    {
        switch (wParam)
        {
        case IDT_TIMER1:
            InvalidateRect(hWnd, NULL, true);
            break;
        }
        // For timer, needs to return 0 in advance
        return 0;
    }
    case WM_DESTROY:
        KillTimer(hWnd, IDT_TIMER1);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}
