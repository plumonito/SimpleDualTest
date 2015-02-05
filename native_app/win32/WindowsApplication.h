/*
 * Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef _WINDOWS_APPLICATION_H
#define _WINDOWS_APPLICATION_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ScanCodes.h"
#include "System.h"
#include "ManagedPtr.h"

namespace System
{

class Application;

namespace Internal
{

class WindowsApplication: public ManagedObject<WindowsApplication>
{
public:
    WindowsApplication(void);
    ~WindowsApplication(void);

    bool openWindow(HINSTANCE hinst, int width, int height, bool showCursor);
    void closeWindow(void);
    void runApplication(managed_ptr<System::Application> const &app);

private:
    static LRESULT WINAPI EventHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    LRESULT eventHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void initKeyboard(void);

    managed_ptr<System::Application> mCurrentApplication;

    HINSTANCE mInstanceHandle;
    HWND mWindowHandle;
    HDC mWindowDCHandle;

    bool mShowMouseCursor;
    int mMouseX, mMouseY, mMouseKeys;
    int mKeyLastPressedCode;

    // debug log level mask
    int mDebugLogLevel;

    System::Key mScanCodeDecodeTable[SC_TOTAL_COUNT];
};

}

}

#endif

