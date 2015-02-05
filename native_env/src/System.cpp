/*
 * Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#include "System.h"
#include "DeviceProxy.h"

/**
 * Checks whether a key was pressed or repeated.
 * @param key - key code
 * @param delay - repeat delay. The delay after a press is twice the value.
 * @return true if the key is pressed, false otherwise
 */
bool System::KeyIsPressed(Key key, int delay)
{
    return Internal::DeviceProxy::GetInstance()->keyIsPressed(key, delay);
}

/**
 * Checks whether a key was pressed
 * @param key - key code
 * @return true if the key is pressed, false otherwise
 */
bool System::KeyIsPressed(Key key)
{
    return Internal::DeviceProxy::GetInstance()->keyIsPressed(key);
}

/**
 * Checks whether a key was released
 * @param key - key code
 * @return true if the key is released, false otherwise
 */
bool System::KeyIsReleased(Key key)
{
    return Internal::DeviceProxy::GetInstance()->keyIsReleased(key);
}

/**
 * Checks whether a key is down
 * @param key - key code
 * @return true if the key is down, false otherwise
 */
bool System::KeyIsDown(Key key)
{
    return Internal::DeviceProxy::GetInstance()->keyIsDown(key);
}

/**
 * Resets key state
 * @param key - key code
 */
void System::KeyReset(Key key)
{
    Internal::DeviceProxy::GetInstance()->keyReset(key);
}

/**
 * Extended keyboard support function. Returns the ASCII code of 
 * last pressed character
 * @return ASCII code of last pressed key
 */
int System::KeyGetLastPressedChar(void)
{
    return Internal::DeviceProxy::GetInstance()->keyGetLastPressedChar();
}

// ==========================================================================
// JOYSTICK
// ==========================================================================

float System::JoystickGet(System::AnalogAxis axis)
{
    // FIXME: implement LUT instead of switch/case
    switch (axis)
    {
    case ThumbstickLeftX:
        return Internal::DeviceProxy::GetInstance()->getAnalogAxis(Internal::DeviceProxy::ThumbstickLeftX);
    case ThumbstickLeftY:
        return Internal::DeviceProxy::GetInstance()->getAnalogAxis(Internal::DeviceProxy::ThumbstickLeftY);
    case ThumbstickRightX:
        return Internal::DeviceProxy::GetInstance()->getAnalogAxis(Internal::DeviceProxy::ThumbstickRightX);
    case ThumbstickRightY:
        return Internal::DeviceProxy::GetInstance()->getAnalogAxis(Internal::DeviceProxy::ThumbstickRightY);
    case TriggerLeft:
        return Internal::DeviceProxy::GetInstance()->getAnalogAxis(Internal::DeviceProxy::TriggerLeft);
    case TriggerRight:
        return Internal::DeviceProxy::GetInstance()->getAnalogAxis(Internal::DeviceProxy::TriggerRight);
    }

    return 0.0f;
}

// ==========================================================================
// TIMER
// ==========================================================================

double System::TimerGetTime(void)
{
    return Internal::DeviceProxy::GetInstance()->getTime();
}

double System::TimerGetFrameTime(void)
{
    return Internal::DeviceProxy::GetInstance()->getFrameTime();
}

// ==========================================================================
// DISPLAY
// ==========================================================================

uint System::ScreenGetWidth(void)
{
    return Internal::DeviceProxy::GetInstance()->getScreenPixelWidth();
}

uint System::ScreenGetHeight(void)
{
    return Internal::DeviceProxy::GetInstance()->getScreenPixelHeight();
}

