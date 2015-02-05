/*
 * Application.h
 *
 *  Created on: Nov 6, 2012
 *      Author: dpajak
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "ManagedPtr.h"
#include "System.h"

namespace System
{

class Application: public ManagedAbstractObject
{
public:
    Application(void)
    {
    }

    ~Application(void)
    {
    }

    /**
     * Key up event handler.
     * @param button - id of the button
     */
    virtual void keyUp(System::Key button)
    {
        (void)button;
    }

    /**
     * Key down event handler.
     * @param button - id of the button
     */
    virtual void keyDown(System::Key button)
    {
        (void)button;
    }

    /**
     * Touch move event handler.
     * @param id - id of the event
     * @param x - x coordinate of the event
     * @param y - y coordinate of the event
     */
    virtual void touchMove(int id, int x, int y)
    {
        (void)id;
        (void)x;
        (void)y;
    }

    /**
     * Touch down event handler.
     * @param id - id of the event
     * @param x - x coordinate of the event
     * @param y - y coordinate of the event
     */
    virtual void touchDown(int id, int x, int y)
    {
        (void)id;
        (void)x;
        (void)y;
    }

    /**
     * Touch up event handler.
     * @param id - id of the event
     * @param x - x coordinate of the event
     * @param y - y coordinate of the event
     */
    virtual void touchUp(int id, int x, int y)
    {
        (void)id;
        (void)x;
        (void)y;
    }

    /**
     * Touch cancel event handler.
     * @param id - id of the event
     * @param x - x coordinate of the event
     * @param y - y coordinate of the event
     */
    virtual void touchCancel(int id, int x, int y)
    {
        (void)id;
        (void)x;
        (void)y;
    }

    /**
     * Called after applet construction. This function can run initial
     * conditional code such as resource allocation and data loading. If
     * this stage fails the function should return false, so that the
     * framework can shutdown the application gracefully.
     * @return true if applet initialization succeeded, false otherwise.
     * If false is returned, the application will quit immediately.
     */
    virtual bool init(void) = 0;

    /**
     * Applet update function. Called for every received event. This
     * function might be called from a different thread than a render
     * thread! In consequence, it might be invoked more or less frequent
     * than <code>draw()</code>. Do not run any rendering code code.
     * The code here should finish execution as fast as possible, in
     * order to enable the OS to process next system messages.
	 * Warning: update will be called before init
	 * @return true if the applet wants to quit, false if it plans
     * to continue running.
     */
    virtual bool update(void) = 0;

    /**
     * Applet draw function. Called every frame.
     */
    virtual void draw(void) = 0;

    /**
     * Called upon applet termination (prior destructor).
     * Use this to free used resources, deallocate buffers, etc.
     */
    virtual void exit(void)
    {
    }

    /**
     * Called when application is about to go into suspend state.
     * Use this function to stop background threads, and free all
     * intermediate resources.
     */
    virtual void suspend(void)
    {
    }

    /**
     * Called when application is returning from the suspend state.
     */
    virtual void resume(void)
    {
    }

    /**
     * Application entry-point. This function should return an instance of
     * <code>System::Application</code> that the framework will later on
     * pass events and control to.
     * @return instance of <code>System::Application</code>
     */
    static managed_ptr<Application> AppInit(void);

protected:

};

}

#endif /* APPLICATION_H_ */
