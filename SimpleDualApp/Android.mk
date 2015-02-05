LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# manually adding opencv 
OPENCV_CAMERA_MODULES  := on
OPENCV_INSTALL_MODULES := on
OPENCV_LIB_TYPE        := STATIC
include $(NVPACK_ROOT)/OpenCV-2.4.8.2-Tegra-sdk/sdk/native/jni/OpenCV.mk

#LOCAL_PATH := $(call my-dir)
LOCAL_MODULE := SimpleDualAndroid

MY_SOURCES := $(wildcard $(LOCAL_PATH)/src/*.cpp)
#MY_SOURCES += $(wildcard $(LOCAL_PATH)/src2/*.cpp)

LOCAL_SRC_FILES  += $(MY_SOURCES:$(LOCAL_PATH)/%=%)

#$(warning "START")

#$(warning $(LOCAL_PATH))
#$(warning $(OPENCV_LOCAL_C_INCLUDES))
#$(warning $(LOCAL_C_INCLUDES))

#$(warning "END")

LOCAL_C_INCLUDES += C:\code\eigen-2.2.4
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/src

#LOCAL_STATIC_LIBRARIES := native_camera $(LOCAL_STATIC_LIBRARIES)  
LOCAL_WHOLE_STATIC_LIBRARIES += native_app

LOCAL_CFLAGS     += -fPIC -DANDROID -fsigned-char

# add base framework
#LOCAL_STATIC_LIBRARIES += libnative_camera

# compiler settings
LOCAL_CFLAGS += -O2 -mtune=cortex-a9 -march=armv7-a -ffast-math -std=gnu++0x
LOCAL_CFLAGS += -DARCH_ARM -DDEBUG_MODE

LOCAL_CPP_FEATURES := rtti

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
  LOCAL_ARM_MODE     := arm
  LOCAL_ARM_NEON     := true
  ARCH_ARM_HAVE_NEON := true
endif

include $(BUILD_SHARED_LIBRARY)

$(call import-add-path,..)
$(call import-module, native_app)
