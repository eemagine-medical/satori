LOCAL_PATH:= $(call my-dir)

LOCAL_CFLAGS := -std=c++14 -fexceptions -frtti -DNDEBUG
LOCAL_EXPORT_CFLAGS := -std=c++14 -fexceptions -frtti -DNDEBUG
TARGET_PLATFORM := android-18
TARGET_ARCH_ABI := armeabi-v7a

include $(LOCAL_PATH)/boost.mk
include $(LOCAL_PATH)/libusb.mk
include $(LOCAL_PATH)/eego-sdk.mk
include $(LOCAL_PATH)/sdk-test1.mk
