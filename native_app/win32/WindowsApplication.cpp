/*
 * Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#include <memory>
#include "WindowsApplication.h"
#include "Device.h"
#include "RenderThread.h"
#include "DeviceProxy.h"

// XXX: this constant might be missing in MinGW Win32 SDK
#ifndef MAPVK_VK_TO_CHAR
#define MAPVK_VK_TO_CHAR 2
#endif

static LPCWSTR gAppWindowClassName = L"rawWindowClass";
static LPCWSTR gAppWindowTitle = L"NVIDIA Test Framework";

System::Internal::WindowsApplication::WindowsApplication(void)
        : mInstanceHandle(0), mWindowHandle(0), mWindowDCHandle(0), mShowMouseCursor(false), mMouseX(0), mMouseY(0),
          mMouseKeys(0), mKeyLastPressedCode(0), mDebugLogLevel(LOG_ALL)
{
    initKeyboard();
}

System::Internal::WindowsApplication::~WindowsApplication(void)
{
    closeWindow();
}

void System::Internal::WindowsApplication::initKeyboard(void)
{
    System::Key *kdata = mScanCodeDecodeTable;

    for (int i = 0; i < SC_TOTAL_COUNT; i++)
    {
        kdata[i] = KeyNone;
    }

    kdata[SC_ESCAPE] = KeyEscape;
    kdata[SC_1] = Key1;
    kdata[SC_2] = Key2;
    kdata[SC_3] = Key3;
    kdata[SC_4] = Key4;
    kdata[SC_5] = Key5;
    kdata[SC_6] = Key6;
    kdata[SC_7] = Key7;
    kdata[SC_8] = Key8;
    kdata[SC_9] = Key9;
    kdata[SC_0] = Key0;
    kdata[SC_MINUS] = KeyMinus;
    kdata[SC_EQUALS] = KeyEquals;
    kdata[SC_BACK] = KeyBackSpace;
    kdata[SC_TAB] = KeyTab;
    kdata[SC_Q] = KeyQ;
    kdata[SC_W] = KeyW;
    kdata[SC_E] = KeyE;
    kdata[SC_R] = KeyR;
    kdata[SC_T] = KeyT;
    kdata[SC_Y] = KeyY;
    kdata[SC_U] = KeyU;
    kdata[SC_I] = KeyI;
    kdata[SC_O] = KeyO;
    kdata[SC_P] = KeyP;
    kdata[SC_LBRACKET] = KeyBracketLeft;
    kdata[SC_RBRACKET] = KeyBracketRight;
    kdata[SC_RETURN] = KeyEnter;
    kdata[SC_LCONTROL] = KeyCtrlLeft;
    kdata[SC_A] = KeyA;
    kdata[SC_S] = KeyS;
    kdata[SC_D] = KeyD;
    kdata[SC_F] = KeyF;
    kdata[SC_G] = KeyG;
    kdata[SC_H] = KeyH;
    kdata[SC_J] = KeyJ;
    kdata[SC_K] = KeyK;
    kdata[SC_L] = KeyL;
    kdata[SC_SEMICOLON] = KeySemicolon;
    kdata[SC_APOSTROPHE] = KeyTilde;
    kdata[SC_LSHIFT] = KeyShiftLeft;
    kdata[SC_BACKSLASH] = KeyBackSlash;
    kdata[SC_Z] = KeyZ;
    kdata[SC_X] = KeyX;
    kdata[SC_C] = KeyC;
    kdata[SC_V] = KeyV;
    kdata[SC_B] = KeyB;
    kdata[SC_N] = KeyN;
    kdata[SC_M] = KeyM;
    kdata[SC_COMMA] = KeyComa;
    kdata[SC_PERIOD] = KeyDot;
    kdata[SC_SLASH] = KeySlash;
    kdata[SC_RSHIFT] = KeyShiftRight;
    kdata[SC_ALT] = KeyAlt;
    kdata[SC_SPACE] = KeySpace;
    kdata[SC_CAPSLOCK] = KeyCapsLock;
    kdata[SC_F1] = KeyF1;
    kdata[SC_F2] = KeyF2;
    kdata[SC_F3] = KeyF3;
    kdata[SC_F4] = KeyF4;
    kdata[SC_F5] = KeyF5;
    kdata[SC_F6] = KeyF6;
    kdata[SC_F7] = KeyF7;
    kdata[SC_F8] = KeyF8;
    kdata[SC_F9] = KeyF9;
    kdata[SC_F10] = KeyF10;
    kdata[SC_F11] = KeyF11;
    kdata[SC_F12] = KeyF12;
    kdata[SC_HOME] = KeyHome;
    kdata[SC_UP] = KeyUp;
    kdata[SC_PAGEUP] = KeyPageUp;
    kdata[SC_SUBTRACT] = KeyMinus;
    kdata[SC_LEFT] = KeyLeft;
    kdata[SC_RIGHT] = KeyRight;
    kdata[SC_END] = KeyEnd;
    kdata[SC_DOWN] = KeyDown;
    kdata[SC_PAGEDOWN] = KeyPageDown;
    kdata[SC_INSERT] = KeyInsert;
    kdata[SC_DELETE] = KeyDelete;
}

LRESULT WINAPI System::Internal::WindowsApplication::EventHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WindowsApplication *instance = reinterpret_cast<WindowsApplication *>(GetWindowLongPtr(hWnd, 0));
    return instance->eventHandler(hWnd, message, wParam, lParam);
}

LRESULT System::Internal::WindowsApplication::eventHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
    case WM_CLOSE:
        PostQuitMessage(0);
        return TRUE;

    case WM_ACTIVATE:
        if (!mShowMouseCursor)
        {
            if (wParam == WA_ACTIVE)
            {
                ShowCursor(0);
            }
            else if (wParam == WA_INACTIVE)
            {
                ShowCursor(1);
            }
        }
        return TRUE;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        mKeyLastPressedCode = (int)wParam;
        // dispatch key down only for non-repeating events
        if ((HIWORD(lParam) & KF_REPEAT) == 0)
        {
            int scanCode = HIWORD(lParam) & 0xff;
            System::Key key = scanCode < SC_TOTAL_COUNT ? mScanCodeDecodeTable[scanCode] : KeyNone;

            if (key != KeyNone)
            {
                System::Internal::DeviceProxy::GetInstance()->dispatchKeyDown(key);
                mCurrentApplication->keyDown(key);
            }
        }
        return TRUE;

    case WM_SYSKEYUP:
    case WM_KEYUP:
    {
        int scanCode = HIWORD(lParam) & 0xff;
        System::Key key = scanCode < SC_TOTAL_COUNT ? mScanCodeDecodeTable[scanCode] : KeyNone;

        if (key != KeyNone)
        {
            System::Internal::DeviceProxy::GetInstance()->dispatchKeyUp(key);
            mCurrentApplication->keyUp(key);
        }
    }
        return TRUE;

    case WM_MOUSEMOVE:
        mMouseX = (int)(lParam & 0xffff);
        mMouseY = (int)(lParam >> 16) & 0xffff;
        return TRUE;

    case WM_RBUTTONDOWN:
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
        mMouseKeys = (int)wParam;
        return TRUE;
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

bool System::Internal::WindowsApplication::openWindow(HINSTANCE hinst, int width, int height, bool showCursor)
{
    mShowMouseCursor = showCursor;
    mInstanceHandle = hinst;

    // init window
    WNDCLASSW wndClass;
    memset(&wndClass, 0, sizeof(WNDCLASSW));

    wndClass.style = CS_BYTEALIGNCLIENT | CS_OWNDC;
    wndClass.lpfnWndProc = EventHandler;
    wndClass.hInstance = hinst;
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.lpszClassName = gAppWindowClassName;
    wndClass.cbWndExtra = sizeof(System::Internal::WindowsApplication *);
    RegisterClassW(&wndClass);

    int const windowFlags = WS_BORDER | WS_DLGFRAME | WS_MINIMIZEBOX | WS_SYSMENU;

    RECT dest;
    dest.left = 0;
    dest.top = 0;
    dest.right = width;
    dest.bottom = height;
    AdjustWindowRectEx(&dest, windowFlags, false, WS_EX_APPWINDOW);

    int const rwidth = dest.right - dest.left;
    int const rheight = dest.bottom - dest.top;
    int const sx = (GetSystemMetrics(SM_CXSCREEN) - rwidth) >> 1;
    int const sy = (GetSystemMetrics(SM_CYSCREEN) - rheight) >> 1;

    mWindowHandle = CreateWindowExW(WS_EX_APPWINDOW, gAppWindowClassName, gAppWindowTitle, windowFlags, sx, sy, rwidth,
                                    rheight, 0, 0, mInstanceHandle, 0);

    if (mWindowHandle == 0)
    {
        return false;
    }

    SetWindowLongPtr(mWindowHandle, 0, reinterpret_cast<LONG_PTR>(this));
    ShowWindow(mWindowHandle, SW_SHOW);
    SetActiveWindow(mWindowHandle);

    // OpenGL part
    mWindowDCHandle = GetDC(mWindowHandle);

    PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
    memset(&pixelFormatDescriptor, 0, sizeof(PIXELFORMATDESCRIPTOR));

    pixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pixelFormatDescriptor.nVersion = 1;
    pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
    pixelFormatDescriptor.cColorBits = 24;
    pixelFormatDescriptor.cDepthBits = 16;
    pixelFormatDescriptor.cStencilBits = 0;
    pixelFormatDescriptor.iLayerType = PFD_MAIN_PLANE;

    int const pixelFormat = ChoosePixelFormat(mWindowDCHandle, &pixelFormatDescriptor);

    if (!pixelFormat)
    {
        LOG_DEBUG( LOG_SYS, "ChoosePixelFormat() failed!");
        return false;
    }

    if (!SetPixelFormat(mWindowDCHandle, pixelFormat, &pixelFormatDescriptor))
    {
        LOG_DEBUG( LOG_SYS, "SetPixelFormat() failed!");
        return false;
    }

    return true;
}

void System::Internal::WindowsApplication::closeWindow(void)
{
    if (mWindowHandle != 0)
    {
        if (mWindowDCHandle != 0)
        {
            ReleaseDC(mWindowHandle, mWindowDCHandle);
            mWindowDCHandle = 0;
        }

        DestroyWindow(mWindowHandle);
        mWindowHandle = 0;
    }

    if (mInstanceHandle != 0)
    {
        UnregisterClassW(gAppWindowClassName, mInstanceHandle);
        mInstanceHandle = 0;
    }
}

// ============================================================================================
// WIN32 MAIN LOOP
// ============================================================================================

static bool ProcessWindowMessageQueue(void)
{
    MSG msg;
    BOOL rval = GetMessage(&msg, 0, 0, 0);
    if (rval < 0)
    {
        // error -> quit
        return false;
    }

    do
    {
        if (msg.message == WM_QUIT)
        {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE));

    return true;
}

void System::Internal::WindowsApplication::runApplication(managed_ptr<System::Application> const &app)
{
    mCurrentApplication = app;

    System::Internal::DeviceProxy *dproxy = System::Internal::DeviceProxy::GetInstance();
    dproxy->setScreenPixelWidth(System::Device::CanvasWidth);
    dproxy->setScreenPixelHeight(System::Device::CanvasHeight);

    // init render thread
    managed_ptr<System::RenderThread> renderThread = new System::RenderThread(mWindowDCHandle, app);
    renderThread->start();

    int prevMouseX = mMouseX;
    int prevMouseY = mMouseY;
    int prevMouseKeys = mMouseKeys;

    while (renderThread->isRunning())
    {
        if (!ProcessWindowMessageQueue())
        {
            // WM_QUIT received
            break;
        }

        // handle keyboard events
        double frameTime = dproxy->getTime();
        dproxy->updateKeyboardState(frameTime);

        if (mKeyLastPressedCode != 0)
        {
            int lkey = MapVirtualKeyW(mKeyLastPressedCode, MAPVK_VK_TO_CHAR) & 0xff;
            if (lkey > 0x80)
            {
                lkey = 0;
            }

            dproxy->keySetLastPressedChar(lkey);
        }

        // fps control
        if (dproxy->keyIsPressed(KeyF2))
        {
            renderThread->setFPS(Device::CanvasMaxFPS);
        }
        else if (dproxy->keyIsPressed(KeyF3))
        {
            renderThread->setFPS(Device::CanvasMaxFPS >> 1);
        }
        else if (dproxy->keyIsPressed(KeyF4))
        {
            renderThread->setFPS(Device::CanvasMaxFPS >> 2);
        }

        // handle mouse events
        int eventFlag = prevMouseKeys ^ mMouseKeys;
        if (eventFlag & 0x1)
        {
            if (mMouseKeys & 0x1)
            {
                app->touchDown(0, mMouseX, mMouseY);
            }
            else
            {
                app->touchUp(0, mMouseX, mMouseY);
            }
        }
        else
        {
            if ((mMouseKeys & 0x1) != 0 && (mMouseX != prevMouseX || mMouseY != prevMouseY))
            {
                app->touchMove(0, mMouseX, mMouseY);
            }
        }

        // execute app loop code
        if (app->update())
        {
            // exit program
            break;
        }

        // clean-up and update mouse & keyboard events
        mKeyLastPressedCode = 0;
        dproxy->keySetLastPressedChar(0);

        prevMouseX = mMouseX;
        prevMouseY = mMouseY;
        prevMouseKeys = mMouseKeys;
    }

    // dereference
    mCurrentApplication.reset();
}

int main(void)
{
    // init native application
    managed_ptr<System::Internal::WindowsApplication> nativeApp(new System::Internal::WindowsApplication());

    //  FILE *log = freopen("log.txt", "wt", stdout);

    if (!nativeApp->openWindow(GetModuleHandle(0), System::Device::CanvasWidth, System::Device::CanvasHeight, true))
    {
        return 0;
    }

    // init app
    managed_ptr<System::Application> app = System::Application::AppInit();
    if (app.get() == 0)
    {
        return 0;
    }

    nativeApp->runApplication(app);

    // dereference app
    app = 0;

    // dereference device proxy components
    System::Internal::DeviceProxy::GetComponents().clear();

    return 0;
}
