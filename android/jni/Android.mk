APP_PLATFORM := android-10
TARGET_PLATFORM := android-10

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../
include jni/libvpx/build/make/Android.mk

#LOCAL_PATH := $(call my-dir)
#include $(CLEAR_VARS)

#MY_FAMRT_PATH := ../../../common/FamRT
#MY_CLIENT_PATH := ../../../client/nclient



#LOCAL_MODULE := libvpx
#LOCAL_SRC_FILES := $(abspath jni/)/libvpx.a
#include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
include $(abspath jni/)/../../thirdparty/libyuv/Android.mk

include $(CLEAR_VARS)
include $(abspath jni/)/../../thirdparty/Codec2Android.mk

include $(CLEAR_VARS)
LOCAL_PATH := $(abspath jni/)
LOCAL_CPP_EXTENSION := .cpp

ifeq ($(NDK_DEBUG),0)
	LOCAL_CFLAGS += -DNDEBUG=1
endif
ifeq ($(NDK_DEBUG),1)
	LOCAL_CFLAGS += -D_DEBUG=1 -g
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../thirdparty/libyuv/include
LOCAL_MODULE    := guruchat
LOCAL_SRC_FILES := guruchat.cpp #$(LOCAL_PATH)/platform/android/guruchat/jni/guruchat.cpp

  #$(MY_CLIENT_PATH)/common/RolEventParser.cpp
  #../../../common/include/classlib.cpp

# for native audio
LOCAL_LDLIBS    += -lOpenSLES -lvpx # -lm -llog -ljnigraphics -lGLESv1_CM -lz
LOCAL_STATIC_LIBRARIES := codec2 libyuv_static

include $(BUILD_SHARED_LIBRARY)
