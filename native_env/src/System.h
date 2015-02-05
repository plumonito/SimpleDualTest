/*
 * Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "SystemCore.h"

namespace System
{

enum Key
{
    KeyA = 0,
    KeyB,
    KeyC,
    KeyD,
    KeyE,
    KeyF,
    KeyG,
    KeyH,
    KeyI,
    KeyJ,
    KeyK,
    KeyL,
    KeyM,
    KeyN,
    KeyO,
    KeyP,
    KeyQ,
    KeyR,
    KeyS,
    KeyT,
    KeyU,
    KeyV,
    KeyW,
    KeyX,
    KeyY,
    KeyZ,
    Key0,
    Key1,
    Key2,
    Key3,
    Key4,
    Key5,
    Key6,
    Key7,
    Key8,
    Key9,
    KeyUp,
    KeyDown,
    KeyLeft,
    KeyRight,
    KeySpace,
    KeyBackSpace,
    KeyEnter,
    KeyInsert,
    KeyDelete,
    KeyHome,
    KeyEnd,
    KeyPageUp,
    KeyPageDown,
    KeyEquals,
    KeyMinus,
    KeySlash,
    KeyBackSlash,
    KeyDot,
    KeyComa,
    KeySemicolon,
    KeyBracketLeft,
    KeyBracketRight,
    KeyCapsLock,
    KeyShiftLeft,
    KeyShiftRight,
    KeyCtrlLeft,
    KeyCtrlRight,
    KeyTilde,
    KeyTab,
    KeyAlt,
    KeyEscape,
    KeyF1,
    KeyF2,
    KeyF3,
    KeyF4,
    KeyF5,
    KeyF6,
    KeyF7,
    KeyF8,
    KeyF9,
    KeyF10,
    KeyF11,
    KeyF12,

    JoystickThumbstickLeft,
    JoystickThumbstickRight,
    JoystickButtonA,
    JoystickButtonB,
    JoystickButtonX,
    JoystickButtonY,
    JoystickButtonL1,
    JoystickButtonR1,
    JoystickButtonStart,
    JoystickPadLeft,
    JoystickPadRight,
    JoystickPadUp,
    JoystickPadDown,

    PhoneKeyLeft,
    PhoneKeyRight,
    PhoneKeyDown,
    PhoneKeyUp,
    PhoneKeyFire,
    PhoneKeySoftLeft,
    PhoneKeySoftRight,
    PhoneKeyHash,
    PhoneKeyStar,
    PhoneKeyClear,
    PhoneKey0,
    PhoneKey1,
    PhoneKey2,
    PhoneKey3,
    PhoneKey4,
    PhoneKey5,
    PhoneKey6,
    PhoneKey7,
    PhoneKey8,
    PhoneKey9,

    KeyNone,
    TotalKeyCodeCount
};

enum AnalogAxis
{
    ThumbstickLeftX, ThumbstickLeftY, ThumbstickRightX, ThumbstickRightY, TriggerLeft, TriggerRight
};

// C-like global access state-less API

// USER INPUT
bool KeyIsPressed(Key key, int delay);
bool KeyIsPressed(Key key);
bool KeyIsReleased(Key key);
bool KeyIsDown(Key key);
void KeyReset(Key key);
int KeyGetLastPressedChar(void);

// JOYSTICK
float JoystickGet(AnalogAxis axis);

// DISPLAY
uint ScreenGetWidth(void);
uint ScreenGetHeight(void);

// TIMER
double TimerGetTime(void);
double TimerGetFrameTime(void);

}

#endif /* SYSTEM_H_ */
