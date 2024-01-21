// Win32_CircleAndLine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdint.h>
#include <time.h>

#define global_variable static
#define internal_function static
#define persistant_local static

#define X_UPPER_BOUND 720
#define Y_UPPER_BOUND 1280
#define RADIUS_UPPER_BOUND 300

struct WindowDimension
{
    int width;
    int height;
};

struct BackBuffer
{
    int width;
    int height;
    int bytesPerPixel;
    void * memory;
    BITMAPINFO bitmapInfo;
    int pitch;
};

global_variable bool globalRunning;
global_variable BackBuffer globalBackBuffer;
global_variable HDC globalDeviceContext;

internal_function WindowDimension
GetWindowDimension(HWND windowHandle)
{
    WindowDimension dimension;
    RECT rect;
    GetClientRect(windowHandle, &rect);
    dimension.width = rect.right - rect.left;
    dimension.height = rect.bottom - rect.top;

    return dimension;
}

internal_function void
CopyBufforToWindow(HWND const * windowHandle)
{
    WindowDimension dimension = GetWindowDimension(*windowHandle);
    StretchDIBits(globalDeviceContext,
                  0, 0, dimension.width, dimension.height,
                  0, 0, globalBackBuffer.width, globalBackBuffer.height,
                  globalBackBuffer.memory, &globalBackBuffer.bitmapInfo,
                  DIB_RGB_COLORS, SRCCOPY);
}

internal_function void
RenderGradient(int xOffset, int yOffset, HWND windowHandle)
{
    uint8_t * row = (uint8_t *)globalBackBuffer.memory;
    for (int y = 0; y < globalBackBuffer.height; ++y)
    {
        uint32_t * pixel = (uint32_t *)row;
        for (int x = 0; x < globalBackBuffer.width; ++x)
        {
            char unsigned blue = x + yOffset;
            char unsigned green = y + xOffset;
            *pixel++ = ((green << 8) | blue);
        }
        row += globalBackBuffer.pitch;
    }

    CopyBufforToWindow(&windowHandle);
}

internal_function void
RenderWhite(int xOffset, int yOffset, HWND windowHandle)
{
    uint8_t * row = (uint8_t *)globalBackBuffer.memory;
    for (int y = 0; y < globalBackBuffer.height; ++y)
    {
        uint32_t * pixel = (uint32_t *)row;
        for (int x = 0; x < globalBackBuffer.width; ++x)
        {
            char unsigned blue = 255;
            char unsigned green = 255;
            char unsigned red = 255;
            *pixel++ = ((red << 16) | (green << 8) | blue);
        }
        row += globalBackBuffer.pitch;
    }

    CopyBufforToWindow(&windowHandle);
}

internal_function void
RenderLine(int x0, int y0,
           int x1, int y1,
           HWND windowHandle)
{
    int slope = (x1 == x0) ? 0 : (y1 - y0) / (x1 - x0);
    int intercept = y0 - slope * x0;

    uint8_t * row = (uint8_t *)globalBackBuffer.memory;
    uint8_t blue = rand() % 255;
    uint8_t green = rand() % 255;
    uint8_t red = rand() % 255;

    // TODO(omid): Optimize buffer traverse
    for (int y = 0; y < globalBackBuffer.height; ++y)
    {
        uint32_t * pixel = (uint32_t *)row;
        for (int x = 0; x < globalBackBuffer.width; ++x)
        {
            if (y == (slope * x + intercept))
            {
                *pixel = ((red << 16) | (green << 8) | blue);
            }
            ++pixel;
        }
        row += globalBackBuffer.pitch;
    }

    CopyBufforToWindow(&windowHandle);
}

internal_function void
RenderCircle(int xCenter, int yCenter,
             int radius,
             HWND windowHandle)
{
    uint8_t * row = (uint8_t *)globalBackBuffer.memory;
    uint8_t blue = rand() % 255;
    uint8_t green = rand() % 255;
    uint8_t red = rand() % 255;

    for (int y = 0; y < globalBackBuffer.height; ++y)
    {
        uint32_t * pixel = (uint32_t *)row;
        for (int x = 0; x < globalBackBuffer.width; ++x)
        {
            if (((y - yCenter) * (y - yCenter) + (x - xCenter) * (x - xCenter)) < (radius * radius))
            {
                *pixel = ((red << 16) | (green << 8) | blue);
            }
            ++pixel;
        }
        row += globalBackBuffer.pitch;
    }

    CopyBufforToWindow(&windowHandle);
}

internal_function void
ResizeDIBSection(int width, int height)
{
    if (globalBackBuffer.memory)
    {
        VirtualFree(globalBackBuffer.memory, 0, MEM_RELEASE);
    }

    globalBackBuffer.width = width;
    globalBackBuffer.height = height;
    globalBackBuffer.bytesPerPixel = 4;

    globalBackBuffer.bitmapInfo.bmiHeader.biSize = sizeof(globalBackBuffer.bitmapInfo.bmiHeader);
    globalBackBuffer.bitmapInfo.bmiHeader.biWidth = globalBackBuffer.width;
    globalBackBuffer.bitmapInfo.bmiHeader.biHeight = -globalBackBuffer.height; // to have top-down bitmap (not bottom-up)
    globalBackBuffer.bitmapInfo.bmiHeader.biPlanes = 1;
    globalBackBuffer.bitmapInfo.bmiHeader.biBitCount = 32;
    globalBackBuffer.bitmapInfo.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = globalBackBuffer.width * globalBackBuffer.height * globalBackBuffer.bytesPerPixel;
    globalBackBuffer.memory = VirtualAlloc(0, BitmapMemorySize,
                                           MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    globalBackBuffer.pitch = width * globalBackBuffer.bytesPerPixel;
}

LRESULT CALLBACK WindowProc(
    HWND   windowHandle,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
)
{
    LRESULT result = 0;

    switch (message)
    {
        case WM_ACTIVATE:
        {
            OutputDebugStringA("Activated\n");
        }break;
        case WM_DESTROY:
        {
            globalRunning = false;
        }break;
        case WM_SIZE:
        {

        }break;
        case WM_CLOSE:
        {
            globalRunning = false;
        }break;
        case WM_SYSKEYDOWN:
            [[fallthrough]];
        case WM_SYSKEYUP:
            [[fallthrough]];
        case WM_KEYDOWN:
            [[fallthrough]];
        case WM_KEYUP:
        {
            uint32_t virtualKeyCode = wParam;

            #define KeyMessageWasDownBit (1 << 30)
            #define KeyMessageIsDownBit (1 << 31)
            bool wasDown = ((lParam & KeyMessageWasDownBit) != 0);
            bool isDown = ((lParam & KeyMessageIsDownBit) == 0);

            if (wasDown != isDown)
            {
                if (virtualKeyCode == 'W')
                {
                    RenderWhite(0, 0, windowHandle);
                }
                else if (virtualKeyCode == 'G')
                {
                    RenderGradient(0, 0, windowHandle);
                }
                else if (virtualKeyCode == 'L')
                {
                    int x0 = rand() % X_UPPER_BOUND;
                    int y0 = rand() % Y_UPPER_BOUND;
                    int x1 = rand() % X_UPPER_BOUND;
                    int y1 = rand() % Y_UPPER_BOUND;
                    RenderLine(x0, y0, x1, y1,
                               windowHandle);
                }
                else if (virtualKeyCode == 'C')
                {
                    int xCenter = rand() % X_UPPER_BOUND;
                    int yCenter = rand() % Y_UPPER_BOUND;
                    int radius = rand() % RADIUS_UPPER_BOUND;
                    RenderCircle(xCenter, yCenter, radius,
                                 windowHandle);
                }
            }

        }break;
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(windowHandle, &paint);

            CopyBufforToWindow(&windowHandle);

            EndPaint(windowHandle, &paint);
        }break;
        default:
        {
            result = DefWindowProc(windowHandle, message, wParam, lParam);
        }break;
    }
    return result;
}

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
            PSTR lpCmdLine, INT nCmdShow)
{
    WNDCLASS windowClass = {};
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = L"My Window Class";

    // NOTE(omid): Create Buffer
    ResizeDIBSection(1280, 720);

    // NOTE(omid): Seed the rng
    srand(time(0));

    if (RegisterClass(&windowClass))
    {
        HWND windowHandle = CreateWindowEx(0, windowClass.lpszClassName,
                                           L"My Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                           CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                           0, 0, hInstance, 0);

        if (windowHandle)
        {
            globalRunning = true;
            MSG message;
            globalDeviceContext = GetDC(windowHandle);

            int xoffset = 0;
            int yoffset = 0;

            while (globalRunning)
            {

                while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
                {
                    if (message.message == WM_QUIT)
                    {
                        globalRunning = false;
                    }

                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                }

                // RenderGradient(xoffset, yoffset, windowHandle);
                // RenderWhite(xoffset, yoffset, windowHandle);
            }
        }

    }

    return 0;
}