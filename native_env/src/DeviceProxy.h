/*
 * Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef DEVICEPROXY_H_
#define DEVICEPROXY_H_

#include <vector>
#include <string>
#include "ManagedPtr.h"
#include "System.h"
#include "SystemTimer.h"

#ifdef ANDROID
#include <jni.h>
struct AAssetManager;
#endif

namespace System
{

namespace Internal
{

// internal system object

class DeviceProxy: public ManagedAbstractObject
{
public:
    enum Components
    {
        ComponentProxyInstance = 0, ComponentCamera, ComponentFileCacheBlockAllocator, TotalComponentCount
    };

    enum AnalogAxis
    {
        ThumbstickLeftX = 0,
        ThumbstickLeftY,
        ThumbstickRightX,
        ThumbstickRightY,
        TriggerLeft,
        TriggerRight,
        TotalAnalogAxisCount
    };

    DeviceProxy(void);
    ~DeviceProxy(void);

    uint getScreenPixelWidth(void);
    void setScreenPixelWidth(uint value);
    uint getScreenPixelHeight(void);
    void setScreenPixelHeight(uint value);

    void updateKeyboardState(double time);
    bool keyIsPressed(System::Key key, int delay);
    bool keyIsPressed(System::Key key);
    bool keyIsReleased(System::Key key);
    bool keyIsDown(System::Key key);
    void keyReset(System::Key key);
    int keyGetLastPressedChar(void);
    void keySetLastPressedChar(int key);

    void dispatchKeyDown(System::Key key);
    void dispatchKeyUp(System::Key key);

    float getAnalogAxis(AnalogAxis id);
    void setAnalogAxis(AnalogAxis id, float value);

    double getTime(void);
    double getFrameTime(void);

    std::string const &getExternalPath(void);
    void setExternalPath(std::string const &path);

#ifdef ANDROID
    AAssetManager *getNativeAssetManager(void);
    void setNativeAssetManager(AAssetManager *instance);

    JNIEnv *getNativeJNIEnvironment(void);
    void setNativeJNIEnvironment(JNIEnv *instance);

    jobject getNativeClassInstance(void);
    void setNativeClassInstance(jobject instance);
#endif

    static std::vector<managed_ptr<ManagedAbstractObject> > &GetComponents(void);
    static DeviceProxy *GetInstance(void);

private:
    Timer mTimer;

    double mLastKeyEventTime;
    double mInterruptTime;
    double mInterruptShift;

    // user input support
    int mKeyLastPressedChar;
    uint mKeyState[System::TotalKeyCodeCount >> 5], mKeyPrevState[System::TotalKeyCodeCount >> 5];
    uint mKeyPressed[System::TotalKeyCodeCount >> 5], mKeyReleased[System::TotalKeyCodeCount >> 5];
    double mKeyPressTime[System::TotalKeyCodeCount], mKeyLastRepeatTime[System::TotalKeyCodeCount];

    float mAnalogAxisValue[TotalAnalogAxisCount];

    std::string mExternalPath;
    uint mScreenWidth;
    uint mScreenHeight;

#ifdef ANDROID
    AAssetManager *mNativeAssetManager;
    JNIEnv *mNativeJNIEnvironment;
    jobject mNativeClassInstance;
#endif
};

}

}

#endif /* DEVICEPROXY_H_ */
