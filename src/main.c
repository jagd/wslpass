#include <Windows.h>

LRESULT CALLBACK MessageCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}


DWORD makeWindow()
{
    WNDCLASS wndClass;
    MSG msg;
    HWND hWnd = NULL;
    HINSTANCE hInstance = GetModuleHandle(NULL);
    ZeroMemory(&wndClass, sizeof(wndClass));
    wndClass.lpfnWndProc = MessageCallback;
    wndClass.hInstance = hInstance;
    wndClass.hCursor = LoadCursor(hInstance, IDC_ARROW);
    wndClass.lpszClassName = TEXT("TemplateWindowClass_RenameThis");

    if (!RegisterClass(&wndClass)) {
        return GetLastError();
    }

    hWnd = CreateWindow(
        wndClass.lpszClassName,
        TEXT("Window Title"),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL);

    while (1) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (DWORD)msg.wParam;
}

UINT mainProxy(LPCTSTR arg)
{
    (void)arg;
    return makeWindow();
}

void mainCRTStartup()
{
    TCHAR* p = GetCommandLine();
    while (*p) {
        if (*p == '\'' || *p == '"') {
            TCHAR quote = *p;
            ++p;
            while (*p && *p != quote)
                ++p;
        }
        ++p;
        if (*p == ' ') {
            while (*p == ' ')
                ++p;
            break;
        }
    }
    ExitProcess(mainProxy(p));
}
