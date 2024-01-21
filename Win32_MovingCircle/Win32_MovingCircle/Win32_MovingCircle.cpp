#include <windows.h>
#include <stdint.h>
#include <math.h> 
#include <time.h>

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

#define global_var static
#define internal_func static
#define local_persist static

typedef struct {
    int height;
    int width;
} WindowDimension;

typedef struct {
    int width;
    int height;
    int pitch;
    int bytesPerPixel;
    void * memory;
    BITMAPINFO info;
} Backbuffer;

global_var bool gRunning = true;
global_var Backbuffer gBackbuffer = {};
global_var HDC gDeviceContext;

internal_func WindowDimension
GetWindowDimension(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    WindowDimension ret;
    ret.width = rect.right - rect.left;
    ret.height = rect.bottom - rect.top;

    return ret;
}

internal_func void
ResizeDibSection(int width, int height) {

    if (gBackbuffer.memory) {
        VirtualFree(gBackbuffer.memory, 0, MEM_RELEASE);
    }

    gBackbuffer.bytesPerPixel = 4;
    gBackbuffer.width = width;
    gBackbuffer.height = height;
    gBackbuffer.pitch = width * gBackbuffer.bytesPerPixel;

    gBackbuffer.info.bmiHeader.biSize = sizeof(gBackbuffer.info.bmiHeader);
    gBackbuffer.info.bmiHeader.biWidth = width;
    gBackbuffer.info.bmiHeader.biHeight = -height;
    gBackbuffer.info.bmiHeader.biPlanes = 1;
    gBackbuffer.info.bmiHeader.biBitCount = 32;
    gBackbuffer.info.bmiHeader.biCompression = BI_RGB;

    gBackbuffer.memory = VirtualAlloc(0, width * height * gBackbuffer.bytesPerPixel, MEM_COMMIT | MEM_RESERVE,
                                      PAGE_READWRITE);
}

internal_func void
CopyBackbufferToWindow(HWND hwnd) {
    WindowDimension window = GetWindowDimension(hwnd);
    StretchDIBits(gDeviceContext,
                  0, 0, window.width, window.height,
                  0, 0, gBackbuffer.width, gBackbuffer.height,
                  gBackbuffer.memory,
                  &gBackbuffer.info, DIB_RGB_COLORS, SRCCOPY);
}

internal_func void
RenderSome() {
    U8 blue = 222;
    U8 green = 222;
    U8 red = 222;

    //U8 * rw = (U8 *)gBackbuffer.memory;
    U32 * px = (U32 *)gBackbuffer.memory;
    for (int y = 0; y < 720; ++y) {
        //U32 * pixel = (U32 *)rw;
        for (int x = 0; x < 1280; ++x) {
            //*pixel++ = (red << 16) | (green << 8) | (blue);
            *px++ = (red << 16) | (green << 8) | (blue);
        }
        //rw += gBackbuffer.pitch;
    }
}

global_var U8 blue = rand() % 255;
global_var U8 green = rand() % 255;
global_var U8 red = rand() % 255;
global_var bool blueincrease = true;

global_var U16 radius = 50;
global_var bool radiusincrease = true;

internal_func void
RenderCircle(int xCenterOffset, int yCenterOffset) {

    if (blueincrease) {
        blue += 2;
    }
    if (225 <= blue) {
        blueincrease = false;
    }
    if (!blueincrease) {
        blue--;
    }
    if (0 >= blue) {
        blueincrease = true;
    }

    U16 xCenter = 0;
    U16 yCenter = 0;

    xCenter += xCenterOffset;
    yCenter += yCenterOffset;

    if (xCenter > gBackbuffer.width) {
        xCenter = 0;
    }
    if (yCenter > gBackbuffer.height) {
        yCenter = 0;
    }
    if (radiusincrease) {
        if (++radius > 255) {
            radius = 255;
            radiusincrease = false;
        }
    }
    else {
        if (--radius <= 0) {
            radius = 0;
            radiusincrease = true;
        }
    }
    U32 * px = (U32 *)gBackbuffer.memory;
    for (int y = 0; y < gBackbuffer.height; ++y) {
        for (int x = 0; x < gBackbuffer.width; ++x) {
            //if ((pow(y - yCenter, 2) + pow(x - xCenter, 2)) < pow(radius, 2)) {

            if (((y - yCenter) * (y - yCenter) + (x - xCenter) * (x - xCenter)) < (radius * radius)) {
                *px = (red << 16) | (green << 8) | (blue);
            }
            ++px;
        }
    }
}

internal_func void
RenderWeirdCircle(int xCenterOffset, int yCenterOffset) {

    if (blueincrease) {
        blue += 2;
    }
    if (225 <= blue) {
        blueincrease = false;
    }
    if (!blueincrease) {
        blue--;
    }
    if (0 >= blue) {
        blueincrease = true;
    }

    U16 xCenter = 0;
    U16 yCenter = 0;

    xCenter += xCenterOffset;
    yCenter += yCenterOffset;

    if (xCenter > gBackbuffer.width) {
        xCenter = 0;
    }
    if (yCenter > gBackbuffer.height) {
        yCenter = 0;
    }

    U32 * px = (U32 *)gBackbuffer.memory;
    for (int y = 0; y < gBackbuffer.height; ++y) {
        for (int x = 0; x < gBackbuffer.width; ++x) {
            //if ((pow(y - yCenter, 2) + pow(x - xCenter, 2)) < pow(radius, 2)) {

            // NOTE(omid): Weird Behaviour 
            if (++radius > 255) {
                radius = 0;
            }

            if (((y - yCenter) * (y - yCenter) + (x - xCenter) * (x - xCenter)) < (radius * radius)) {
                *px = (red << 16) | (green << 8) | (blue);
            }
            ++px;
        }
    }
}

LRESULT CALLBACK MainWindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
) {
    LRESULT ret = 0;
    switch (uMsg) {
        case WM_DESTROY:
        {
            gRunning = false;
        }break;
        default:
        {
            ret = DefWindowProcA(hwnd, uMsg, wParam, lParam);
        }break;
    }
    return ret;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

    srand(time(0));

    WNDCLASSA wndclass = {};
    wndclass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = MainWindowProc;
    wndclass.hInstance = hInstance;
    wndclass.lpszClassName = "Wind Class";

    if (RegisterClassA(&wndclass)) {
        ResizeDibSection(1280, 720);
        HWND hwnd = CreateWindowExA(0,
                                    wndclass.lpszClassName,
                                    "Window 2",
                                    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                    0, 0, 1280, 720, 0, 0, hInstance, 0);
        if (hwnd) {
            int xoffset = 0;
            int yoffset = 0;
            gRunning = true;
            MSG msg;
            gDeviceContext = GetDC(hwnd);
            while (gRunning) {
                while (PeekMessageA(&msg, hwnd, 0, 0, PM_REMOVE)) {
                    if (msg.message == WM_QUIT) {
                        gRunning = false;
                    }
                    TranslateMessage(&msg);
                    DispatchMessageA(&msg);
                }
                RenderSome();
                RenderCircle(xoffset, yoffset);
                xoffset += 1;
                if (xoffset > gBackbuffer.width) {
                    xoffset = 0;
                }
                yoffset += 1;
                if (yoffset > gBackbuffer.height) {
                    yoffset = 0;
                }
                CopyBackbufferToWindow(hwnd);
            }
        }
    }
}
