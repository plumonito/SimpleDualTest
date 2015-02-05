#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <android/log.h>
#include <android/input.h>
#include <nv_and_util/nv_native_app_glue.h>
#include <nv_egl_util/nv_egl_util.h>
#include <GLES2/gl2.h>
#include <sys/stat.h>
#include <errno.h>
#include <memory>
#include "AndroidApplication.h"
#include "Application.h"
#include "DeviceProxy.h"

static int MakeDir(std::string const &path, mode_t mode)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
    {
        if (mkdir(path.c_str(), mode) != 0 && errno != EEXIST)
        {
            return false;
        }
    }
    else if (!S_ISDIR( st.st_mode ))
    {
        errno = ENOTDIR;
        return false;
    }

    return true;
}

static bool MakePath(std::string const &path, mode_t mode)
{
    std::string cstr(path);

    int cpos = 0;
    int ppos = 0;
    bool success = true;
    while (success && (cpos = cstr.find_first_of("/", ppos)) != std::string::npos)
    {
        if (cpos != ppos)
        {
            /* neither root nor double slash in path */
            cstr[cpos] = '\0';
            success = MakeDir(cstr, mode);
            cstr[cpos] = '/';
        }
        ppos = cpos + 1;
    }

    if (success)
    {
        success = MakeDir(path, mode);
    }

    return success;
}

System::Internal::AndroidApplication::AndroidApplication(android_app *app, NvEGLUtil *egl)
        : mNativeAppInstance(app), mEgl(egl), mCurrentApplication(0), mCurrentApplicationState(Initialization),
          mPreviousDigitalCrossState(0)
{
    app->userData = this;
    app->onAppCmd = HandleCmd;
    app->onInputEvent = HandleInput;
}

System::Internal::AndroidApplication::~AndroidApplication(void)
{
}

void System::Internal::AndroidApplication::HandleCmd(android_app *app, int32_t cmd)
{
    static_cast<AndroidApplication *>(app->userData)->handleCommand(cmd);
}

void System::Internal::AndroidApplication::setupButtonMapping(void)
{
    for (int i = 0; i < MaximumKeyCodeValue; i++)
    {
        mButtonMapping[i] = System::KeyNone;
    }

    mButtonMapping[AKEYCODE_BUTTON_THUMBL] = System::JoystickThumbstickLeft;
    mButtonMapping[AKEYCODE_BUTTON_THUMBR] = System::JoystickThumbstickRight;
    mButtonMapping[AKEYCODE_BUTTON_A] = System::JoystickButtonA;
    mButtonMapping[AKEYCODE_BUTTON_B] = System::JoystickButtonB;
    mButtonMapping[AKEYCODE_BUTTON_X] = System::JoystickButtonX;
    mButtonMapping[AKEYCODE_BUTTON_Y] = System::JoystickButtonY;
    mButtonMapping[AKEYCODE_BUTTON_L1] = System::JoystickButtonL1;
    mButtonMapping[AKEYCODE_BUTTON_R1] = System::JoystickButtonR1;
    mButtonMapping[AKEYCODE_BUTTON_START] = System::JoystickButtonStart;
    mButtonMapping[AKEYCODE_DPAD_UP] = System::JoystickPadUp;
    mButtonMapping[AKEYCODE_DPAD_DOWN] = System::JoystickPadDown;
    mButtonMapping[AKEYCODE_DPAD_LEFT] = System::JoystickPadLeft;
    mButtonMapping[AKEYCODE_DPAD_RIGHT] = System::JoystickPadRight;
}

void System::Internal::AndroidApplication::handleCommand(int cmd)
{
    switch (cmd)
    {
    case APP_CMD_INIT_WINDOW:
    case APP_CMD_WINDOW_RESIZED:
        mEgl->setWindow(mNativeAppInstance->window);
        break;

    case APP_CMD_TERM_WINDOW:
        mEgl->setWindow(NULL);
        break;

    case APP_CMD_GAINED_FOCUS:
        if (mCurrentApplication != 0)
        {
            // FIXME: broken resume event support, have to add full app life-cycle FSM
            //mCurrentApplication->resume();
        }
        break;

    case APP_CMD_LOST_FOCUS:
    case APP_CMD_PAUSE:
        if (mCurrentApplication != 0)
        {
            // FIXME: broken resume event support, have to add full app life-cycle FSM
            //mCurrentApplication->suspend();
        }
        break;
    }
}

/**
 * Process the next input event.
 */
int System::Internal::AndroidApplication::HandleInput(android_app *app, AInputEvent *event)
{
    return static_cast<AndroidApplication *>(app->userData)->handleInput(event);
}

int System::Internal::AndroidApplication::handleInput(AInputEvent *event)
{
    //We only handle motion events (touchscreen) and key (button/key) events
    if (mCurrentApplication == 0)
    {
        return 0;
    }

    int eventType = AInputEvent_getType(event);
    System::Internal::DeviceProxy *deviceProxy = System::Internal::DeviceProxy::GetInstance();

    if (eventType == AINPUT_EVENT_TYPE_MOTION)
    {
        if (AInputEvent_getSource(event) == AINPUT_SOURCE_JOYSTICK)
        {
            float vx, vy;

            vx = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_X, 0);
            vy = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Y, 0);
            deviceProxy->setAnalogAxis(System::Internal::DeviceProxy::ThumbstickLeftX, vx);
            deviceProxy->setAnalogAxis(System::Internal::DeviceProxy::ThumbstickLeftY, vy);

            vx = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Z, 0);
            vy = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RZ, 0);
            deviceProxy->setAnalogAxis(System::Internal::DeviceProxy::ThumbstickRightX, vx);
            deviceProxy->setAnalogAxis(System::Internal::DeviceProxy::ThumbstickRightY, vy);

            vx = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_LTRIGGER, 0);
            vy = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RTRIGGER, 0);
            deviceProxy->setAnalogAxis(System::Internal::DeviceProxy::TriggerLeft, vx);
            deviceProxy->setAnalogAxis(System::Internal::DeviceProxy::TriggerRight, vy);

            // we convert dpad events into button events
            vx = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_X, 0);
            vy = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_Y, 0);

            int digitalCrossState = 0;
            digitalCrossState |= vx < 0.0f ? 0x1 : 0;
            digitalCrossState |= vx > 0.0f ? 0x2 : 0;
            digitalCrossState |= vy < 0.0f ? 0x4 : 0;
            digitalCrossState |= vy > 0.0f ? 0x8 : 0;

            // decompose state into pressed and released state events
            int pressedState = digitalCrossState & (digitalCrossState ^ mPreviousDigitalCrossState);
            int releasedState = mPreviousDigitalCrossState & (mPreviousDigitalCrossState ^ digitalCrossState);
            mPreviousDigitalCrossState = digitalCrossState;

            if ((pressedState & 0x1) != 0)
            {
                deviceProxy->dispatchKeyDown(System::JoystickPadLeft);
                mCurrentApplication->keyDown(System::JoystickPadLeft);
            }

            if ((pressedState & 0x2) != 0)
            {
                deviceProxy->dispatchKeyDown(System::JoystickPadRight);
                mCurrentApplication->keyDown(System::JoystickPadRight);
            }

            if ((pressedState & 0x4) != 0)
            {
                deviceProxy->dispatchKeyDown(System::JoystickPadUp);
                mCurrentApplication->keyDown(System::JoystickPadUp);
            }

            if ((pressedState & 0x8) != 0)
            {
                deviceProxy->dispatchKeyDown(System::JoystickPadDown);
                mCurrentApplication->keyDown(System::JoystickPadDown);
            }

            if ((releasedState & 0x1) != 0)
            {
                deviceProxy->dispatchKeyUp(System::JoystickPadLeft);
                mCurrentApplication->keyUp(System::JoystickPadLeft);
            }

            if ((releasedState & 0x2) != 0)
            {
                deviceProxy->dispatchKeyUp(System::JoystickPadRight);
                mCurrentApplication->keyUp(System::JoystickPadRight);
            }

            if ((releasedState & 0x4) != 0)
            {
                deviceProxy->dispatchKeyUp(System::JoystickPadUp);
                mCurrentApplication->keyUp(System::JoystickPadUp);
            }

            if ((releasedState & 0x8) != 0)
            {
                deviceProxy->dispatchKeyUp(System::JoystickPadDown);
                mCurrentApplication->keyUp(System::JoystickPadDown);
            }
        }
        else
        {
            int actionData = AMotionEvent_getAction(event);
            int action = actionData & AMOTION_EVENT_ACTION_MASK;

            int pcount = AMotionEvent_getPointerCount(event);
            switch (action)
            {
            case AMOTION_EVENT_ACTION_DOWN:
                for (int32_t i = 0; i < pcount; i++)
                {
                    int mx = AMotionEvent_getX(event, i);
                    int my = AMotionEvent_getY(event, i);
                    mCurrentApplication->touchDown(i, mx, my);
                }
                break;
            case AMOTION_EVENT_ACTION_UP:
                for (int32_t i = 0; i < pcount; i++)
                {
                    int mx = AMotionEvent_getX(event, i);
                    int my = AMotionEvent_getY(event, i);
                    mCurrentApplication->touchUp(i, mx, my);
                }
                break;
            case AMOTION_EVENT_ACTION_MOVE:
                for (int32_t i = 0; i < pcount; i++)
                {
                    int mx = AMotionEvent_getX(event, i);
                    int my = AMotionEvent_getY(event, i);
                    mCurrentApplication->touchMove(i, mx, my);
                }
                break;
            case AMOTION_EVENT_ACTION_CANCEL:
                for (int32_t i = 0; i < pcount; i++)
                {
                    int mx = AMotionEvent_getX(event, i);
                    int my = AMotionEvent_getY(event, i);
                    mCurrentApplication->touchCancel(i, mx, my);
                }
                break;
            }
        }

        return 1;
    }
    else if (eventType == AINPUT_EVENT_TYPE_KEY)
    {
        int code = AKeyEvent_getKeyCode(event);
        if (code < MaximumKeyCodeValue)
        {
            // run only for valid key codes
            if (code == AKEYCODE_BACK)
            {
                mCurrentApplicationState = PrepareForExit;
                return 1;
            }

            System::Key buttonId = mButtonMapping[code];

            // TODO: add AKEY_EVENT_ACTION_MULTIPLE support
            // XXX: prevent from reporting repeating down/up events
            if (buttonId != System::KeyNone && AKeyEvent_getRepeatCount(event) == 0)
            {
                int action = AKeyEvent_getAction(event);
                if (action == AKEY_EVENT_ACTION_UP)
                {
                    deviceProxy->dispatchKeyUp(buttonId);
                    mCurrentApplication->keyUp(buttonId);
                    return 1;
                }
                else if (action == AKEY_EVENT_ACTION_DOWN)
                {
                    deviceProxy->dispatchKeyDown(buttonId);
                    mCurrentApplication->keyDown(buttonId);
                    return 1;
                }
            }
        }
    }

    return 0;
}

void System::Internal::AndroidApplication::mainLoop(void)
{
    // initialize proxy device component list
    System::Internal::DeviceProxy::GetComponents().clear();
    System::Internal::DeviceProxy::GetComponents().resize(System::Internal::DeviceProxy::TotalComponentCount);

    System::Internal::DeviceProxy *deviceProxy = System::Internal::DeviceProxy::GetInstance();
    deviceProxy->setNativeAssetManager(mNativeAppInstance->activity->assetManager);
    deviceProxy->setNativeJNIEnvironment(mNativeAppInstance->appThreadEnv);
    deviceProxy->setNativeClassInstance(mNativeAppInstance->appThreadThis);

    setupButtonMapping();

    std::string externalPath(mNativeAppInstance->activity->externalDataPath);
    if (MakePath(externalPath, S_IRWXU))
    {
        LOG_DEBUG( LOG_IO, "external data path: %s", externalPath.c_str());
    }
    else
    {
        LOG_DEBUG( LOG_IO, "error creating external data path: %s", externalPath.c_str());
    }

    externalPath.append(1, '/');
    deviceProxy->setExternalPath(externalPath);

    // init app
    managed_ptr<System::Application> application(System::Application::AppInit());

    mCurrentApplication = application.get();
    mCurrentApplicationState = Initialization;

    while (nv_app_status_running(mNativeAppInstance))
    {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source *source;

        // If not rendering, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident = ALooper_pollAll((nv_app_status_focused(mNativeAppInstance) ? 1 : 250), NULL, &events,
                                        (void **)&source))
               >= 0)
        {
            // If we timed out, then there are no pending messages.
            if (ident == ALOOPER_POLL_TIMEOUT)
            {
                break;
            }

            // Process this event.
            if (source != NULL)
            {
                source->process(mNativeAppInstance, source);
            }

            // Check if we are exiting.  If so, dump out.
            if (!nv_app_status_running(mNativeAppInstance))
            {
                break;
            }
        }

        // FIXME: extract the state machine to a separate function
        // add simulate full application life-cycle that can be only
        // modified by incoming native activity commands

        if (nv_app_status_interactable(mNativeAppInstance))
        {
            if (mEgl->isReadyToRender(true))
            {
                if (mEgl->checkWindowResized())
                {
                    int width = mEgl->getWidth();
                    int height = mEgl->getHeight();

                    System::Internal::DeviceProxy *deviceProxy = System::Internal::DeviceProxy::GetInstance();
                    deviceProxy->setScreenPixelWidth(width);
                    deviceProxy->setScreenPixelHeight(height);

                    glViewport(0, 0, width, height);

                    LOG( "EGL window resize (%ix%i)", width, height);
                }

                // part of the application state machine that is render state dependent
                switch (mCurrentApplicationState)
                {
                case Initialization:
                    if (mCurrentApplication == 0)
                    {
                        mCurrentApplicationState = PrepareForExit;
                    }
                    else
                    {
                        if (mCurrentApplication->init())
                        {
                            mCurrentApplicationState = MainLoop;
                        }
                        else
                        {
                            mCurrentApplicationState = PrepareForExit;
                        }
                    }
                    break;

                case MainLoop:
                {
                    // handle keyboard events
                    double frameTime = deviceProxy->getTime();
                    deviceProxy->updateKeyboardState(frameTime);

                    if (mCurrentApplication->update())
                    {
                        mCurrentApplicationState = PrepareForExit;
                        break;
                    }

                    mCurrentApplication->draw();
                    mEgl->swap();
                    break;
                }

                default:
                    // unknown state -> NOP
                    break;
                }
            }
        }

        // part of the application state machine that is render state dependent
        switch (mCurrentApplicationState)
        {
        case PrepareForExit:
            // we have to make sure to clean up while we still have EGL context current
            if (mCurrentApplication != 0)
            {
                application->exit();
                // dereference application instance
                mCurrentApplication = 0;
                application = 0;
            }

            // TODO: check ref count before deallocation (should be 1)
            // dereference device proxy components
            System::Internal::DeviceProxy::GetComponents().clear();

            ANativeActivity_finish(mNativeAppInstance->activity);
            mCurrentApplicationState = Exit;
            break;

        default:
            // unknown state -> NOP
            break;
        }
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(android_app *androidApp)
{
    // Make sure glue isn't stripped.
    app_dummy();

    //    volatile bool __dbg = true;
    //    while( __dbg )
    //    {
    //    }

    std::unique_ptr<NvEGLUtil> egl(NvEGLUtil::create());
    if (egl == 0)
    {
        // if we have a basic EGL failure, we need to exit immediately; nothing else we can do
        nv_app_force_quit_no_cleanup(androidApp);
        return;
    }

    // initialize android application support class
    managed_ptr<System::Internal::AndroidApplication> instance(
            new System::Internal::AndroidApplication(androidApp, egl.get()));

    // run main application loop
    instance->mainLoop();
}
