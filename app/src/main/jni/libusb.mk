include $(CLEAR_VARS)
LOCAL_CFLAGS := -std=c++14 -fexceptions -frtti -DNDEBUG
LOCAL_EXPORT_CFLAGS := -std=c++14 -fexceptions -frtti -DNDEBUG
LOCAL_MODULE    := libusb
LOCAL_SRC_FILES := \
../../../../edi-framework/3rdParty/rtl_tcp_andro-/jni/libusb-andro/libusb/core.c \
../../../../edi-framework/3rdParty/rtl_tcp_andro-/jni/libusb-andro/libusb/descriptor.c \
../../../../edi-framework/3rdParty/rtl_tcp_andro-/jni/libusb-andro/libusb/hotplug.c \
../../../../edi-framework/3rdParty/rtl_tcp_andro-/jni/libusb-andro/libusb/io.c \
../../../../edi-framework/3rdParty/rtl_tcp_andro-/jni/libusb-andro/libusb/sync.c \
../../../../edi-framework/3rdParty/rtl_tcp_andro-/jni/libusb-andro/libusb/os/linux_usbfs.c \
../../../../edi-framework/3rdParty/rtl_tcp_andro-/jni/libusb-andro/libusb/os/linux_netlink.c \
../../../../edi-framework/3rdParty/rtl_tcp_andro-/jni/libusb-andro/libusb/os/poll_posix.c \
../../../../edi-framework/3rdParty/rtl_tcp_andro-/jni/libusb-andro/libusb/os/threads_posix.c
LOCAL_LDLIBS    := -llog
LOCAL_EXPORT_C_INCLUDES := ../edi-framework/3rdParty/rtl_tcp_andro-/jni/libusb-andro ../edi-framework/3rdParty/rtl_tcp_andro-/jni/libusb-andro/libusb
LOCAL_C_INCLUDES        := ../edi-framework/3rdParty/rtl_tcp_andro-/jni/libusb-andro ../edi-framework/3rdParty/rtl_tcp_andro-/jni/libusb-andro/libusb
include $(BUILD_SHARED_LIBRARY)
