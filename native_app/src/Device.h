/*
 * Device.h
 *
 *  Created on: Oct 23, 2012
 *      Author: dpajak
 */

#ifndef DEVICE_H_
#define DEVICE_H_

namespace System
{

namespace Device
{

enum
{
//    CanvasWidth = 1552, CanvasHeight = 800, CanvasMaxFPS = 60
    // CanvasWidth = 1280, CanvasHeight = 960, CanvasMaxFPS = 60
    // CanvasWidth = 1024, CanvasHeight = 768, CanvasMaxFPS = 60
//    CanvasWidth = 1366,
//    CanvasHeight = 720,
    CanvasWidth = 1280,
    CanvasHeight = 768,
    CanvasMaxFPS = 60
};

}

}

#ifdef ANDROID
// copied from http://www.khronos.org/registry/gles/extensions/EXT/EXT_texture_rg.txt
#define GL_RED       0x1903
#define GL_RG        0x8227
#endif

#endif /* DEVICE_H_ */
