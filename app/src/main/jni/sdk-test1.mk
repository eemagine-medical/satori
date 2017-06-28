include $(CLEAR_VARS)
LOCAL_C_INCLUDES       := ../edi-framework/sdk ../edi-framework/3rdParty/libeep/src ../edi-framework/3rdParty/boost_android ../edi-framework/include
LOCAL_C_EXPORT_INCLUDES       := ../edi-framework/sdk ../edi-framework/3rdParty/libeep/src ../edi-framework/3rdParty/boost_android ../edi-framework/include
LOCAL_CFLAGS           := -DLIBEEP_VERSION_MAJOR="0" -DLIBEEP_VERSION_MINOR="0" -DLIBEEP_VERSION_MICRO="0" -DLIBEEP_VERSION_PATCH="0" -std=c++14 -fexceptions -frtti
LOCAL_CPPFLAGS         := -DEEGO_SDK_BIND_STATIC -std=c++14 -fexceptions -frtti
LOCAL_LDLIBS           := -landroid -lEGL -lGLESv2
LOCAL_SHARED_LIBRARIES := eego-SDK
LOCAL_MODULE           := satori
LOCAL_STATIC_LIBRARIES := android_native_app_glue
LOCAL_DISABLE_FORMAT_STRING_CHECKS := true
LOCAL_SRC_FILES        := \
../../../../edi-framework/sdk/eemagine/sdk/wrapper.cc \
sdk-helper.cc \
gl-helper.cc \
sdk-test1.cc \
../../../../edi-framework/3rdParty/libeep/src/libavr/avr.c \
../../../../edi-framework/3rdParty/libeep/src/libcnt/cnt.c \
../../../../edi-framework/3rdParty/libeep/src/libcnt/evt.c \
../../../../edi-framework/3rdParty/libeep/src/libcnt/raw3.c \
../../../../edi-framework/3rdParty/libeep/src/libcnt/riff64.c \
../../../../edi-framework/3rdParty/libeep/src/libcnt/riff.c \
../../../../edi-framework/3rdParty/libeep/src/libcnt/seg.c \
../../../../edi-framework/3rdParty/libeep/src/libcnt/trg.c \
../../../../edi-framework/3rdParty/libeep/src/libeep/eepio.c \
../../../../edi-framework/3rdParty/libeep/src/libeep/eepmem.c \
../../../../edi-framework/3rdParty/libeep/src/libeep/eepmisc.c \
../../../../edi-framework/3rdParty/libeep/src/libeep/eepraw.c \
../../../../edi-framework/3rdParty/libeep/src/libeep/val.c \
../../../../edi-framework/3rdParty/libeep/src/libeep/var_string.c \
../../../../edi-framework/3rdParty/libeep/src/v4/eep.c
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
