#include "a_menu.h"
#include <stdbool.h>
#include <stdint.h>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

typedef struct Window {
    int32_t displayWidth;
    int32_t displayHeight;
    int32_t minimizeWidth;
    int32_t minimizeHeight;

    bool fullscreen;
    bool allowToggleFullscreen;

    WNDCLASSEXW class;
    MSG message;

    void* handle;
    void* deviceContext;
} Window;

static void InitialiseWindow(Window* window, HINSTANCE instanceHandle);
static void InitialiseRenderer(BITMAPINFO* bitmapInfo, uint32_t** bitmapData);
static void UpdateInputs(void* windowHandle, uint8_t* keyboardState, float* cursorPositionX,
                         float* cursorPositionY);
static void UpdateWindowChanges(Window* window, bool* shouldWindowResize, uint8_t* keyboardState);

static const uint8_t keyExit = 0x1B;             // Esc
static const uint8_t keyToggleFullscreen = 0x7A; // F11

static const int32_t defaultInternalWidth = 640;
static const int32_t defaultInternalHeight = 400;

bool shouldProgramRun = true;
bool shouldWindowResize = false;

int WINAPI wWinMain(HINSTANCE instanceHandle, HINSTANCE previousInstanceHandle, LPWSTR commandLine,
                    int commandShow) {
    void* mutex = CreateMutexW(NULL, FALSE, L"MyMinigame");
    (void)mutex;

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return EXIT_FAILURE;
    }

    Window window;
    InitialiseWindow(&window, instanceHandle);

    if (RegisterClassExW(&window.class) == 0) {
        return EXIT_FAILURE;
    }

    window.handle = CreateWindowExW(0, window.class.lpszClassName, L"Minigames",
                                    WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                                    window.displayWidth, window.displayHeight, NULL, NULL,
                                    instanceHandle, NULL);

    if (window.handle == NULL) {
        return EXIT_FAILURE;
    }

    BITMAPINFO bitmapInfo;
    uint32_t* bitmapData;
    InitialiseRenderer(&bitmapInfo, &bitmapData);

    uint8_t keyboardState[256];
    float cursorPositionX;
    float cursorPositionY;

    InitialiseApp(bitmapData, bitmapInfo.bmiHeader.biWidth, bitmapInfo.bmiHeader.biHeight);

    while (shouldProgramRun) {
        while (PeekMessageW(&window.message, window.handle, 0, 0, PM_REMOVE)) {
            DispatchMessageW(&window.message);
        }

        UpdateInputs(window.handle, keyboardState, &cursorPositionX, &cursorPositionY);
        cursorPositionX *= bitmapInfo.bmiHeader.biWidth;
        cursorPositionY *= bitmapInfo.bmiHeader.biHeight;

        UpdateWindowChanges(&window, &shouldWindowResize, keyboardState);

        UpdateApp(keyboardState, cursorPositionX, cursorPositionY);

        window.deviceContext = GetDC(window.handle);
        StretchDIBits(window.deviceContext, 0, window.displayHeight, window.displayWidth,
                      -window.displayHeight, 0, 0, bitmapInfo.bmiHeader.biWidth,
                      bitmapInfo.bmiHeader.biHeight, bitmapData, &bitmapInfo, DIB_RGB_COLORS,
                      SRCCOPY);
        ReleaseDC(window.handle, window.deviceContext);

        Sleep(1);
    }

    return EXIT_SUCCESS;
}

LRESULT CALLBACK WindowProc(HWND windowHandle, UINT message, WPARAM wParameter, LPARAM lParameter) {
    switch (message) {
    case WM_CLOSE:
        shouldProgramRun = false;
        DestroyWindow(windowHandle);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        shouldWindowResize = true;
        return 0;
    }

    return DefWindowProcW(windowHandle, message, wParameter, lParameter);
}

static void InitialiseWindow(Window* gameWindow, HINSTANCE instanceHandle) {
    gameWindow->displayWidth = defaultInternalWidth * 2;
    gameWindow->displayHeight = defaultInternalHeight * 2;
    gameWindow->minimizeWidth = defaultInternalWidth * 2;
    gameWindow->minimizeHeight = defaultInternalHeight * 2;

    gameWindow->fullscreen = false;
    gameWindow->allowToggleFullscreen = true;

    gameWindow->class.cbSize = sizeof(WNDCLASSEXW);
    gameWindow->class.style = 0;
    gameWindow->class.lpfnWndProc = WindowProc;
    gameWindow->class.cbClsExtra = 0;
    gameWindow->class.cbWndExtra = 0;
    gameWindow->class.hInstance = instanceHandle;
    gameWindow->class.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    gameWindow->class.hCursor = LoadCursorW(NULL, IDC_ARROW);
    gameWindow->class.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    gameWindow->class.lpszMenuName = L"MinigamesMenuName";
    gameWindow->class.lpszClassName = L"MinigamesClassName";
    gameWindow->class.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
}

static void InitialiseRenderer(BITMAPINFO* bitmapInfo, uint32_t** bitmapData) {
    bitmapInfo->bmiHeader.biSize = sizeof(bitmapInfo->bmiHeader);
    bitmapInfo->bmiHeader.biWidth = defaultInternalWidth;
    bitmapInfo->bmiHeader.biHeight = defaultInternalHeight;
    bitmapInfo->bmiHeader.biBitCount = 32;
    bitmapInfo->bmiHeader.biCompression = BI_RGB;
    bitmapInfo->bmiHeader.biPlanes = 1;

    *bitmapData =
        malloc(sizeof(uint32_t) * bitmapInfo->bmiHeader.biWidth * bitmapInfo->bmiHeader.biHeight);
}

static void UpdateInputs(void* windowHandle, uint8_t* keyboardState, float* cursorPositionX,
                         float* cursorPositionY) {
    GetKeyboardState(keyboardState);
    for (int32_t i = 0; i < 256; i++) {
        keyboardState[i] &= 0x80;
    }

    POINT cursorScreenPosition;
    GetCursorPos(&cursorScreenPosition);
    RECT clientRect;
    GetClientRect(windowHandle, &clientRect);

    ScreenToClient(windowHandle, &cursorScreenPosition);
    *cursorPositionX = (float)cursorScreenPosition.x / clientRect.right;
    *cursorPositionY = (float)cursorScreenPosition.y / clientRect.bottom;
}

static void UpdateWindowChanges(Window* window, bool* shouldWindowResize, uint8_t* keyboardState) {
    if (*shouldWindowResize) {
        RECT windowRectangle;
        GetWindowRect(window->handle, &windowRectangle);

        window->displayWidth = windowRectangle.right - windowRectangle.left;
        window->displayHeight = windowRectangle.bottom - windowRectangle.top;
        *shouldWindowResize = false;
    }

    if (keyboardState[keyToggleFullscreen] && window->allowToggleFullscreen) {
        MONITORINFO monitorInfo;
        monitorInfo.cbSize = sizeof(MONITORINFO);
        GetMonitorInfoW(MonitorFromWindow(window->handle, MONITOR_DEFAULTTOPRIMARY), &monitorInfo);

        if (window->fullscreen) {
            window->displayWidth = window->minimizeWidth;
            window->displayHeight = window->minimizeHeight;
            SetWindowLongPtrW(window->handle, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
        } else {
            window->displayWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
            window->displayHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
            SetWindowLongPtrW(window->handle, GWL_STYLE, WS_VISIBLE);
        }

        SetWindowPos(window->handle, HWND_TOP, monitorInfo.rcMonitor.left,
                     monitorInfo.rcMonitor.top, window->displayWidth, window->displayHeight,
                     SWP_SHOWWINDOW);
        window->fullscreen = !window->fullscreen;
    }

    window->allowToggleFullscreen = !keyboardState[keyToggleFullscreen];

    if (keyboardState[keyExit]) {
        PostMessage(window->handle, WM_CLOSE, 0, 0);
    }
}
