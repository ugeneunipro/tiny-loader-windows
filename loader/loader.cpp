// loader.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "loader.h"

#define MAX_LOADSTRING 100

#define SZ_0    TEXT("Unipro UGENE Online Installer is loading...")
#define SZ_1    TEXT("Unipro UGENE installer preparing   ")
#define SZ_2    TEXT("Unipro UGENE installer preparing.  ")
#define SZ_3   TEXT("Unipro UGENE installer preparing.. ")
#define SZ_4   TEXT("Unipro UGENE installer preparing...")
#define SZ_CLEAN   TEXT("                                   ")
#define SZ_SPLASH  TEXT("Splash window")

#define SPLASH_WIDTH  500
#define SPLASH_HEIGHT  50
HBRUSH hSplashBrush;

#define ID_TIMER_1	0x1111
#define ID_TIMER_2	0x1112
#define ID_TIMER_3	0x1113
#define ID_TIMER_4	0x1114

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
int   AlreadyRunMessageBox();

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void downloadFile(int *isDownloaded) {
    printf_s("test");
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

double progress = 0.0;
TCHAR szProgress[120] = { 0 };

int my_progress_func(void *bar,
    double t, /* dltotal */
    double d, /* dlnow */
    double ultotal,
    double ulnow)
{
    char *sz = (char *)malloc(120 * sizeof(char));
    progress = d <= 0.01 || t <= 0.01 ? 0.0 : d / t;
    sprintf_s(sz, 119, "Downloaded  %.2f %%", d <= 0.01 || t <= 0.01 ? 0.0 : progress * 100);

    size_t wn = 0;
    mbsrtowcs_s(&wn, NULL, 0, (const char **)&sz, 0, NULL);
    mbsrtowcs_s(&wn, szProgress, 119, (const char **)&sz, wn + 1, NULL);

    return 0;
}

void downloadInstaller(const char *link, const char *outFileName, int* isDownloaded) {
    CURL *curl_handle;
    FILE *outFile;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init(); //init the curl session
    curl_easy_setopt(curl_handle, CURLOPT_URL, link); //set URL to get here
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);  //Switch on full protocol/debug output while testing. Only for debug purpose.

    if (false) {
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L); //disable progress meter, set to 0L to enable and disable debug output
    } else {
        /* Switch on full protocol/debug output */
        curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, my_progress_func);
    }

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data); //send all data to this function
    curl_easy_setopt(curl_handle, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    fopen_s(&outFile, outFileName, "wb"); //open the file
    if (outFile) {
        CURLcode res;
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, outFile); //write the page body to this file handle
        res = curl_easy_perform(curl_handle); //get it!
        fclose(outFile); //close the header file
        if (res != CURLE_OK) {
            MessageBox(0, _T("Cannot download Unipro UGENE installer."), _T("Error"), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
            exit(1);
        }
    }
    curl_easy_cleanup(curl_handle); //cleanup curl stuff
    *isDownloaded = 1;
}

int numBitsSystem() {
    // We don't need to catch all the CPU architectures in this function;
    // only those where the host CPU might be different than the build target
    // (usually, 64-bit platforms).
    SYSTEM_INFO info;
    GetNativeSystemInfo(&info);
    switch (info.wProcessorArchitecture) {
#  ifdef PROCESSOR_ARCHITECTURE_AMD64
    case PROCESSOR_ARCHITECTURE_AMD64:
        return 64;
#  endif
#  ifdef PROCESSOR_ARCHITECTURE_IA32_ON_WIN64
    case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
#  endif
    case PROCESSOR_ARCHITECTURE_IA64:
        return 64;
    case PROCESSOR_ARCHITECTURE_INTEL:
        return 32;
    }
    return 32;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPTSTR    lpCmdLine,
    _In_ int       nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    MSG msg;
    HACCEL hAccelTable;

    lstrcpy(szWindowClass, TEXT("UgeneTinyInstallerSplashWindow"));
    HWND hWndExists = FindWindow(szWindowClass, NULL);
    if (hWndExists != NULL) {
        AlreadyRunMessageBox();
        return -1;
    }

    // Initialize global strings
    hSplashBrush = CreateSolidBrush(RGB(73, 104, 118));
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    ATOM a = MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LOADER));

    //Get temporary dir and set path for output file
    TCHAR buf[MAX_PATH];
    if (GetTempPath(MAX_PATH, buf) == 0)
        MessageBox(0, buf, _T("Temp path"), 0);
    TCHAR fileName[MAX_PATH] = _T("ugeneInstaller.exe");
    TCHAR _outFileName[MAX_PATH];
    _stprintf_s(_outFileName, _T("%s%s"), buf, fileName);
    int len = wcslen(_outFileName);
    char* outFileName = (char*)_alloca(len + 1);
    size_t charsConverted = 0;
    wcstombs_s(&charsConverted, outFileName, len + 1, _outFileName, len + 1);
    //Check number bits of system
    const char *link;
    int isDownloaded = 0;
    if (numBitsSystem() == 32) {
        static const char *linkP = "http://ugene.net/downloads/installer_windows_x32.exe";
        link = linkP;
    } else {
        static const char *linkP = "http://ugene.net/downloads/installer_windows_x64.exe";
        link = linkP;
    }
    //Downloading file in different thread
    std::thread first(downloadInstaller, link, outFileName, &isDownloaded);

    // Main message loop:
    bool isAlreadySended = false;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (isDownloaded == 1 && isAlreadySended != true) {
            isAlreadySended = true;
            first.join();
            PostMessage(msg.hwnd, WM_DESTROY, 0, 0);
        }
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    //Execute downloaded installer
    WinExec(outFileName, SW_SHOW);
    printf_s("finish");
    return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = NULL;

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    HWND hWnd;
    RECT rect;
    int  splashwidth = SPLASH_WIDTH;
    int  splashheight = SPLASH_HEIGHT * 3;

    hInst = hInstance; // Store instance handle in our global variable
    SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID)&rect, 0);

    //   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
    //      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    hWnd = CreateWindowEx(WS_EX_TOOLWINDOW,
        szWindowClass,
        NULL,
        //WS_OVERLAPPED,
        WS_POPUP | WS_VISIBLE,
        (rect.right - rect.left - splashwidth) / 2,
        (rect.bottom - rect.top - splashheight) / 2,
        splashwidth,
        splashheight,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!hWnd) {
        return FALSE;
    }
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_NCCALCSIZE:  //CAPTURE THIS MESSAGE AND RETURN NULL
        return NULL;
    case WM_CREATE:
        //SetTimer(hWnd, ID_TIMER_1, 3000, NULL);
        SetTimer(hWnd, ID_TIMER_2, 1000, NULL);
        SetTimer(hWnd, ID_TIMER_3, 2000, NULL);
        SetTimer(hWnd, ID_TIMER_4, 3000, NULL);
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps = { 0 };
        RECT rect = { 0 };
        HDC hDC = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &rect);
        Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);
        SetTextAlign(hDC, TA_CENTER);
        TEXTMETRIC* tm = NULL;
        LONG fontHeight = 20;
        if (GetTextMetrics(hDC, tm)) {
            fontHeight = tm->tmAscent + tm->tmDescent;
        }

        TextOut(hDC, rect.left + (rect.right - rect.left) / 2, rect.top + 17, SZ_1, lstrlen(SZ_1));
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        KillTimer(hWnd, ID_TIMER_1);
        KillTimer(hWnd, ID_TIMER_2);
        KillTimer(hWnd, ID_TIMER_3);
        KillTimer(hWnd, ID_TIMER_4);
        CloseWindow(hWnd);
        PostQuitMessage(0);
        break;
    case WM_TIMER:
    {
        HDC hDC = GetDC(hWnd);
        SetTextAlign(hDC, TA_CENTER | VTA_CENTER);
        RECT rect = { 0 };

        GetClientRect(hWnd, &rect);
        KillTimer(hWnd, wParam);
        switch (wParam) {
        case ID_TIMER_4:
            SetTimer(hWnd, ID_TIMER_1, 1000, NULL);
            SetTimer(hWnd, ID_TIMER_2, 2000, NULL);
            SetTimer(hWnd, ID_TIMER_3, 3000, NULL);
            SetTimer(hWnd, ID_TIMER_4, 4000, NULL);
            Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);
        case ID_TIMER_1:
        case ID_TIMER_2:
        case ID_TIMER_3:
            TextOut(hDC, rect.left + (rect.right - rect.left) / 2, rect.top + 17, SZ_0, lstrlen(SZ_0));
            RECT barF = { rect.left + SPLASH_HEIGHT
                , rect.top + SPLASH_HEIGHT + SPLASH_HEIGHT * 0.2
                , rect.right - SPLASH_HEIGHT
                , rect.top + SPLASH_HEIGHT + SPLASH_HEIGHT - SPLASH_HEIGHT * 0.2 };
            RECT barC = { barF.left
                , barF.top
                , barF.left + (barF.right - barF.left) * progress
                , barF.bottom };
            Rectangle(hDC, barF.left, barF.top, barF.right, barF.bottom);
            FillRect(hDC, &barC, hSplashBrush);
            TextOut(hDC, rect.left + (rect.right - rect.left) / 2, rect.top + 17 + SPLASH_HEIGHT * 2, szProgress, lstrlen(szProgress));
            break;
        }
        ReleaseDC(hWnd, hDC);
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int AlreadyRunMessageBox()
{
    int msgboxID = MessageBox(
        NULL,
        (LPCWSTR)L"Unipro UGENE Online Installer is already started.\nSee progress of the loading in another window.",
        (LPCWSTR)L"Unipro UGENE Installer is Already Started",
        MB_ICONWARNING | MB_APPLMODAL | MB_OK
        );

    switch (msgboxID) {
    case IDCANCEL:
        // TODO: add code
        break;
    case IDTRYAGAIN:
        // TODO: add code
        break;
    case IDCONTINUE:
        // TODO: add code
        break;
    }

    return msgboxID;
}