include $(CLEAR_VARS)
LOCAL_C_INCLUDES       := ../edi-framework/3rdParty/boost_android
LOCAL_CFLAGS           := -DBOOST_ALL_NO_LIB -DBOOST_ALL_DYN_LINK -std=c++14
LOCAL_MODULE           := boost-system
LOCAL_SRC_FILES        := \
../../../../edi-framework/3rdParty/boost_android/libs/system/src/error_code.cpp
include $(BUILD_STATIC_LIBRARY)
