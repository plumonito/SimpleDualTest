LOCAL_PATH := $(call my-dir)

# module definition
include $(CLEAR_VARS)
LOCAL_MODULE := native_app

LOCAL_CFLAGS := -Wall -Wcast-align -Wextra -std=gnu++0x -DARCH_ARM -DDEBUG_MODE

ifeq ($(NDK_DEBUG),1)
  LOCAL_CFLAGS += -DDEBUG
else
  LOCAL_CFLAGS += -O2 -mtune=cortex-a9 -march=armv7-a -ffast-math
endif

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
  LOCAL_ARM_NEON     := true
  ARCH_ARM_HAVE_NEON := true
endif

MY_SOURCES := $(wildcard $(LOCAL_PATH)/src/*.cpp)
MY_SOURCES += $(wildcard $(LOCAL_PATH)/android/*.cpp)

LOCAL_SRC_FILES         := $(MY_SOURCES:$(LOCAL_PATH)/%=%)
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/src
LOCAL_C_INCLUDES        += $(LOCAL_PATH)/android
LOCAL_STATIC_LIBRARIES  := libnative_env nv_and_util nv_egl_util
LOCAL_SHARED_LIBRARIES  := libsys_support

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/src
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/android
LOCAL_EXPORT_LDLIBS     := -lGLESv2 -lEGL

include $(BUILD_STATIC_LIBRARY)

# Add the folder with the NVIDIA helper
#$(call import-add-path, $(NVPACK_PATH)/TDK_Samples/tegra_android_native_samples_v10p10/libs/jni)
#$(call import-add-path, $(NVPACK_PATH)/TDK_Samples/tegra_android_native_samples_v10p03/libs/jni)
$(call import-add-path, $(NVPACK_PATH)/Samples/TDK_Samples/libs/jni)

# Import shared modules
$(call import-module, nv_and_util)
$(call import-module, nv_egl_util)
$(call import-module, native_env)
