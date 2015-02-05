/*
 * Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#include "SystemCore.h"
//#include "math/FastMath.h"
#include "FrameTimer.h"
#include <mmsystem.h>
#include <cstdlib>

#define FRAME_TIMER_DEFAULT_FPS 60

System::FrameTimer::FrameTimer(void)
{
    mFrameTickEvent = CreateEvent(0, FALSE, FALSE, 0);
    mThreadExitEvent = CreateEvent(0, FALSE, FALSE, 0);

    mHasStarted = false;
    setFPS(FRAME_TIMER_DEFAULT_FPS);
}

System::FrameTimer::~FrameTimer(void)
{
    // stop() is blocking but even if the frame thread is still running
    // there is no chance of dead-lock and it will quit almost immediately
    // within timer tick
    stop();

    CloseHandle (mThreadExitEvent);
    CloseHandle (mFrameTickEvent);
}

bool System::FrameTimer::isRunning(void)
{
    return mHasStarted;
}

#ifdef _WIN32

// ============================================================================================
// WIN32 FRAME DRAW REQUEST TIMING CONTROL
// ============================================================================================

// TODO: add synchronized access to methods to improve multi-threading support

void System::FrameTimer::waitForTick( void )
{
    if( mHasStarted )
    {
        WaitForSingleObject( mFrameTickEvent, INFINITE );
    }
}

void System::FrameTimer::setFPS( int maxFPS )
{
    mPeriod = 10000000 / maxFPS; // in 100ns units
}

void System::FrameTimer::stop( void )
{
    if( mHasStarted )
    {
        SetEvent( mThreadExitEvent );
        WaitForSingleObject( mThreadHandle, INFINITE );
        CloseHandle( mThreadHandle );

        timeEndPeriod( mAccuracy );

        mHasStarted = false;

        LOG_DEBUG( LOG_SYS, "this: 0x%08x", (uint)this );
    }
}

void System::FrameTimer::start( void )
{
    if( mHasStarted )
    {
        return;
    }

    uint accuracy = mPeriod / 20000;
    if( accuracy > 10 )
    {
        accuracy = 10;
    }

    TIMECAPS tc;
    timeGetDevCaps( &tc, sizeof(TIMECAPS) );
    if( accuracy < tc.wPeriodMin )
    {
        accuracy = tc.wPeriodMin;
    }

    if( accuracy > tc.wPeriodMax )
    {
        accuracy = tc.wPeriodMax;
    }

    mAccuracy = accuracy;
    timeBeginPeriod( accuracy );

    ResetEvent( mThreadExitEvent );
    ResetEvent( mFrameTickEvent );
    mThreadHandle = CreateThread( 0, 0, NativeThreadProc, this, 0, 0 );

    mHasStarted = true;

    LOG_DEBUG( LOG_SYS, "this: 0x%08x", (uint)this );
}

void System::FrameTimer::run( void )
{
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );

    uint period = mPeriod;
    uint periodHi = period / 10000;
    uint periodLo = period % 10000;

    int maxDelay = period / 2000;

    uint nextTimeHi = timeGetTime() + periodHi;
    uint nextTimeLo = periodLo;

    for(;; )
    {
        uint currentTime = timeGetTime();
        int delta = nextTimeHi - currentTime;

        if( delta > 0 )
        {
            DWORD result;
            if( delta > maxDelay )
            {
                result = WaitForSingleObject( mThreadExitEvent, maxDelay );
            }
            else
            {
                result = WaitForSingleObject( mThreadExitEvent, delta );
            }

            if( result != WAIT_TIMEOUT )
            {
                // time to quit!
                break;
            }
        }

        if( std::abs( delta ) > maxDelay )
        {
            nextTimeHi = currentTime + periodHi;
            nextTimeLo = periodLo;
        }
        else
        {
            nextTimeHi += periodHi;
            nextTimeLo += periodLo;

            if( nextTimeLo >= 10000 )
            {
                nextTimeLo -= 10000;
                nextTimeHi++;
            }
        }

        SetEvent( mFrameTickEvent );

        if( period != mPeriod )
        {
            // max fps changed -> update
            period = mPeriod;
            periodHi = period / 10000;
            periodLo = period % 10000;

            maxDelay = period / 2000;

            nextTimeHi = currentTime + periodHi;
            nextTimeLo = periodLo;
        }
    }
}

DWORD WINAPI System::FrameTimer::NativeThreadProc( void *ptr )
{
    FrameTimer *instance = static_cast<FrameTimer *>( ptr );
    instance->run();
    return 0;
}
#endif

