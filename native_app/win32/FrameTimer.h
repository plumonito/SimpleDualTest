/*
 * Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef FRAMETIMER_H_
#define FRAMETIMER_H_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "Base.h"
#include "ManagedPtr.h"

namespace System
{

class FrameTimer: public ManagedObject<FrameTimer>
{
public:
    FrameTimer(void);
    ~FrameTimer(void);

    void start(void);
    void stop(void);

    bool isRunning(void);

    void waitForTick(void);

    void setFPS(int maxFPS);

private:
    void run(void);

#ifdef _WIN32
    static DWORD WINAPI NativeThreadProc( void *ptr );

    HANDLE mThreadHandle;
    HANDLE mThreadExitEvent;
    HANDLE mFrameTickEvent;
    uint mPeriod;   // in 100ns units
    uint mAccuracy;// in ms units
#endif
    volatile bool mHasStarted;
};

}

#endif /* FRAMETIMER_H_ */
