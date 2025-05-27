#include <windows.h>
#include <shellapi.h>
#include <string>
#include <cstdio>
#include <cstring>
#include <heapapi.h>

#define STOP_ARG "BOOM!"

BOOL CreateProcessWithBlockDllPolicy(LPSTR cmdLine, DWORD* pid, HANDLE* hProcess, HANDLE* hThread) {
    STARTUPINFOEXA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};
    SIZE_T size;

    InitializeProcThreadAttributeList(NULL, 1, 0, &size);
    auto attr = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, size);
    InitializeProcThreadAttributeList(attr, 1, 0, &size);

    DWORD64 policy = PROCESS_CREATION_MITIGATION_POLICY_BLOCK_NON_MICROSOFT_BINARIES_ALWAYS_ON;
    UpdateProcThreadAttribute(attr, 0, PROC_THREAD_ATTRIBUTE_MITIGATION_POLICY, &policy, sizeof(policy), NULL, NULL);

    si.lpAttributeList = attr;
    si.StartupInfo.dwFlags = EXTENDED_STARTUPINFO_PRESENT;

    CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, EXTENDED_STARTUPINFO_PRESENT, NULL, NULL, &si.StartupInfo, &pi);

    DeleteProcThreadAttributeList(attr);
    HeapFree(GetProcessHeap(), 0, attr);
    return TRUE;
}



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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow) {
    if (strcmp(lpCmdLine, STOP_ARG) != 0) {
        char filename[MAX_PATH] = {0};
        if (!GetModuleFileNameA(NULL, filename, sizeof(filename))) return -1;

        size_t cmdLen = strlen(filename) + strlen(STOP_ARG) + 4;
        char* cmdLine = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cmdLen);
        if (!cmdLine) return -1;

        sprintf_s(cmdLine, cmdLen, "\"%s\" %s", filename, STOP_ARG);
        DWORD pid; HANDLE hProc, hThread;

        if (CreateProcessWithBlockDllPolicy(cmdLine, &pid, &hProc, &hThread)) {
            CloseHandle(hProc);
            CloseHandle(hThread);
        }

        HeapFree(GetProcessHeap(), 0, cmdLine);
        return 0;
    }

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
