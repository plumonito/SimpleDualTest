/*
 * Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#include <GL/glew.h>
#include <GL/wglew.h>
#include "RenderThread.h"
#include "Device.h"
#include "SystemCore.h"

System::RenderThread::RenderThread(HDC windowDC, managed_ptr<Application> const &app)
{
    mWindowDrawContext = windowDC;
    mApplication = app;
    mHasStarted = false;
    mIsRunning = false;

    mThreadExitEvent = CreateEvent(0, FALSE, FALSE, 0);
}

System::RenderThread::~RenderThread(void)
{
    stop();

    CloseHandle(mThreadExitEvent);
}

bool System::RenderThread::isRunning(void)
{
    return mHasStarted && mIsRunning;
}

void System::RenderThread::setFPS(int maxFPS)
{
    if (mHasStarted)
    {
        mFrameTimer.setFPS(maxFPS);
    }
}

void System::RenderThread::start(void)
{
    if (mHasStarted)
    {
        return;
    }

    mIsRunning = true;

    ResetEvent(mThreadExitEvent);
    mThreadHandle = CreateThread(0, 0, NativeThreadProc, this, 0, 0);

    mHasStarted = true;

    LOG_DEBUG(LOG_SYS, "this: 0x%08x", (uint)this);
}

void System::RenderThread::stop(void)
{
    if (mHasStarted)
    {
        mIsRunning = false;

        WaitForSingleObject(mThreadExitEvent, INFINITE);
        // for some reason NVIDIA drivers prefer us to terminate the thread instead of a graceful exit
        // XXX: on WinXP and WinServer 2003 this leads to memory leak (thread's stack mem is not freed)
        TerminateThread(mThreadHandle, 0);
        CloseHandle(mThreadHandle);

        mHasStarted = false;

        LOG_DEBUG(LOG_SYS, "this: 0x%08x", (uint)this);
    }
}

static bool IsSupported(char const *name)
{
    if (glewIsSupported(name))
    {
        LOG_DEBUG(LOG_SYS, "%s not supported!", name);
        return false;
    }

    return true;
}

void System::RenderThread::run(void)
{
    //SetThreadAffinityMask(GetCurrentThread(), 0x01);
    // XXX: PointGrey camera hack, we need to lower priority to minimize frame drops
    //SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);

    // gpu info
    LOG_DEBUG(LOG_INFO, "host renderer: %s", glGetString(GL_RENDERER));

    // init GLEW
    glewInit();

    // check extensions required for WGL context reconfiguration (for GLES2 simulation)
    if (!IsSupported("WGL_ARB_create_context") || !IsSupported("WGL_ARB_create_context_profile")
        || !IsSupported("WGL_EXT_create_context_es2_profile"))
    {
        return;
    }

    // change WGL context to GLES2
    int const attriblist[] =
    { WGL_CONTEXT_MAJOR_VERSION_ARB, 2, WGL_CONTEXT_MINOR_VERSION_ARB, 0, WGL_CONTEXT_PROFILE_MASK_ARB,
      WGL_CONTEXT_ES2_PROFILE_BIT_EXT, 0, 0 };

    HGLRC rc = wglCreateContextAttribsARB(mWindowDrawContext, NULL, attriblist);
    if (!rc)
    {
        LOG_DEBUG(LOG_SYS, "failed to initialize GLES2 context!");
        return;
    }

    // replace old GL2 with GLES2 context
    wglMakeCurrent(mWindowDrawContext, NULL);
    wglDeleteContext(mGLRenderingContext);

    mGLRenderingContext = rc;
    if (!wglMakeCurrent(mWindowDrawContext, mGLRenderingContext))
    {
        LOG_DEBUG(LOG_SYS, "wglMakeCurrent() failed!");
        return;
    }

    // check for required extensions
    if (!IsSupported("ARB_texture_rectangle") || !IsSupported("EXT_framebuffer_object"))
    {
        return;
    }

    // disable vsync
    wglSwapIntervalEXT(0);

    if (!mApplication->init())
    {
        return;
    }

    // init render timer thread
    mFrameTimer.setFPS(Device::CanvasMaxFPS);
    mFrameTimer.start();

    // main render loop
    while (mIsRunning)
    {
        mFrameTimer.waitForTick();
        mApplication->draw();

        SwapBuffers(mWindowDrawContext);
    }

    // stop render timer thread
    mFrameTimer.stop();

    // notify the application about termination
    mApplication->exit();
}

DWORD WINAPI System::RenderThread::NativeThreadProc(void *ptr)
{
    RenderThread *instance = static_cast<RenderThread *>(ptr);

    // create render thread GL context
    instance->mGLRenderingContext = wglCreateContext(instance->mWindowDrawContext);
    if (instance->mGLRenderingContext == NULL)
    {
        LOG_DEBUG(LOG_SYS, "wglCreateContext() failed!");
    }
    else
    {
        // attach OpenGL context to render thread
        if (!wglMakeCurrent(instance->mWindowDrawContext, instance->mGLRenderingContext))
        {
            LOG_DEBUG(LOG_SYS, "wglMakeCurrent() failed!");
        }
        else
        {
            // render loop
            instance->run();
            wglMakeCurrent(instance->mWindowDrawContext, NULL);
        }

        wglDeleteContext(instance->mGLRenderingContext);
    }

    // render loop is not running anymore
    instance->mIsRunning = false;

    // for some reason NVIDIA drivers prefer us to terminate the thread instead of a graceful exit
    SetEvent(instance->mThreadExitEvent);
    SuspendThread(GetCurrentThread());

    return 0;
}

