/*
 * Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef RENDERTHREAD_H_
#define RENDERTHREAD_H_

#include "Base.h"
#include "Application.h"
#include "FrameTimer.h"

namespace System
{
class RenderThread: public ManagedObject<RenderThread>
{
public:
    RenderThread(HDC windowDC, managed_ptr<Application> const &app);
    ~RenderThread(void);

    void start(void);
    void stop(void);

    bool isRunning(void);

    void setFPS(int maxFPS);

private:
    void run(void);

    static DWORD WINAPI NativeThreadProc( void *ptr );
    HANDLE mThreadHandle;
    HANDLE mThreadExitEvent;
    HDC mWindowDrawContext;
    HGLRC mGLRenderingContext;

    volatile bool mHasStarted;
    volatile bool mIsRunning;
    managed_ptr<Application> mApplication;
    FrameTimer mFrameTimer;
};

}

#endif /* RENDERTHREAD_H_ */
