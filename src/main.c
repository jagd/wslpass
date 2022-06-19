/* Currently using WriteFile(), which cannot directly write unicode string */
#ifdef UNICODE
#undef UNICODE 
#endif

#include <Windows.h>

/** Max chars including traling zero */
#define MAX_MESSAGE_CHARS 2048
#define MAX_PASSWD_CHARS 2048
#define MASK_WINDOW_CLASS TEXT("PassPrompt")
#define MASK_WND_BG_COLOR RGB(94, 39, 80)
#define ID_OK 100
#define ID_CANCEL 101

LRESULT CALLBACK DlgEditCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LONG_PTR chain;
    switch (uMsg) {
    case WM_CHAR:
        switch (wParam) {
        case VK_ESCAPE:
            PostQuitMessage(0);
            return 0;
        case VK_RETURN:
            SendMessage(GetParent(hWnd), WM_COMMAND, ID_OK, (LPARAM)hWnd);
            break;
        }
        break;
    }
    chain = GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (chain == 0) {
        return 0;
    }
    return CallWindowProc((WNDPROC)chain, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK callback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CHAR:
        switch (wParam) {
        case VK_ESCAPE:
            DestroyWindow(hWnd);
            return 0;
        case VK_RETURN:
            SendMessage(hWnd, WM_COMMAND, ID_OK, (LPARAM)hWnd);
            break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(1);
        return 1;
    case WM_COMMAND:
        if (wParam == ID_CANCEL) {
            DWORD bytesWritten;
            WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), "\n", 1, &bytesWritten, NULL);
            PostQuitMessage(1);
            return 1;
        }
        else if (wParam == ID_OK) {
            LPTSTR buf = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TCHAR) * MAX_PASSWD_CHARS+2);
            HWND hEdit = (HWND)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            const int len = GetWindowText(hEdit, buf, MAX_PASSWD_CHARS)+3;
            buf[len-1] = (TCHAR)'\0';
            buf[len-2] = (TCHAR)'\n';
            buf[len-3] = (TCHAR)'\n';
            if (buf && len > 3) {
                DWORD bytesWritten;
                WriteFile( /* TODO: THIS DOES NOT HANDLE UNICODE STRINGS! */
                    GetStdHandle(STD_OUTPUT_HANDLE),
                    buf, len-1,
                    &bytesWritten, NULL);
                ZeroMemory(buf, sizeof(TCHAR) * MAX_PASSWD_CHARS);
                PostQuitMessage(0);
            }
            else {
                SetFocus(hEdit);
            }
            HeapFree(GetProcessHeap(), 0, buf);
            return 0;
        }
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

DWORD registerWindow()
{
    WNDCLASS wc;

    HINSTANCE hInst = GetModuleHandle(NULL);

    ZeroMemory(&wc, sizeof(wc));

    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(hInst, IDC_ARROW);
    wc.lpfnWndProc = callback;
    wc.lpszClassName = MASK_WINDOW_CLASS;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClass(&wc)) {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}


#define FRAME_WIDTH 320
#define PASSWORD_EDIT_WIDTH FRAME_WIDTH
#define PASSWORD_EDIT_HEIGHT 25
#define BUTTON_HEIGHT 30
#define BUTTON_WIDTH 100
#define VERTICAL_PADDING 10
#define HORIZONTAL_PADDING 20

HWND createWindows(LPCTSTR msg)
{
    RECT rc;
    HWND hEdit = NULL;
    HINSTANCE hInst = GetModuleHandle(NULL);
    HWND hWnd = CreateWindow(
        MASK_WINDOW_CLASS,
        TEXT("Enter Password"),
        WS_POPUP | WS_MAXIMIZE | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInst,
        NULL);

    if (hWnd == NULL) {
        return NULL;
    }

    GetWindowRect(hWnd, &rc);
    hEdit = CreateWindow(
        TEXT("EDIT"),
        TEXT(""),
        WS_BORDER | WS_CHILD | WS_VISIBLE | ES_LEFT | ES_PASSWORD,
        (rc.right - rc.left) / 2 - PASSWORD_EDIT_WIDTH / 2,
        (rc.bottom - rc.top) / 2 - PASSWORD_EDIT_HEIGHT / 2,
        PASSWORD_EDIT_WIDTH, PASSWORD_EDIT_HEIGHT,
        hWnd, NULL, hInst, NULL);

    if (hEdit == NULL) {
        return NULL;
    }
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)hEdit);
    SetWindowLongPtr(hEdit, GWLP_USERDATA, SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)DlgEditCallback));

    GetWindowRect(hEdit, &rc);
    if (NULL == CreateWindow(
        TEXT("STATIC"),
        (msg[0] == '\0') ? TEXT("Enter password:") : msg,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
        rc.left, rc.top - VERTICAL_PADDING - 60, FRAME_WIDTH, 60,
        hWnd, (HMENU)ID_CANCEL, hInst, NULL)) {
        return NULL;
    }
    if (NULL == CreateWindow(
        TEXT("BUTTON"),
        TEXT("&OK"),
        WS_BORDER | WS_CHILD | WS_VISIBLE | BS_VCENTER | BS_CENTER | BS_DEFPUSHBUTTON,
        (rc.right + rc.left) / 2 - HORIZONTAL_PADDING / 2 - BUTTON_WIDTH,
        rc.bottom + VERTICAL_PADDING,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        hWnd, (HMENU)ID_OK, hInst, NULL)) {
        return NULL;
    }
    if (NULL == CreateWindow(
        TEXT("BUTTON"),
        TEXT("&CANCEL"),
        WS_BORDER | WS_CHILD | WS_VISIBLE | BS_VCENTER | BS_CENTER | BS_PUSHBUTTON,
        (rc.right + rc.left) / 2 + HORIZONTAL_PADDING / 2,
        rc.bottom + VERTICAL_PADDING,
        BUTTON_WIDTH, BUTTON_HEIGHT,
        hWnd, (HMENU)ID_CANCEL, hInst, NULL)) {
        return NULL;
    }

    SetFocus(hEdit);
    return hWnd;
}

DWORD mainProxy(LPCTSTR arg)
{
    MSG msg;
    HWND hWnd;
    DWORD error;

    (void)arg;

    error = registerWindow();
    if (error != ERROR_SUCCESS) {
        return error;
    }

    hWnd = createWindows(arg);

    if (hWnd == NULL) {
        return 1;
    }

    while (1) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return(DWORD)msg.wParam;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
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

#ifndef NDEBUG
int main() {
    mainCRTStartup();
    return 0;  /* unreachable */
}
#endif
