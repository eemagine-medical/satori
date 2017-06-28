include $(CLEAR_VARS)
LOCAL_C_INCLUDES       := ../edi-framework/include ../edi-framework/source ../edi-framework/include/shared ../edi-framework/3rdParty/boost_android
LOCAL_CFLAGS           := -DEDI_HAVE_ANDROID -DEDI_HAVE_LIBUSB -std=c++14
LOCAL_SHARED_LIBRARIES := libusb
LOCAL_MODULE           := edi-core
LOCAL_SRC_FILES        := \
../../../../edi-framework/source/driver/core/battery_handler.cpp \
../../../../edi-framework/source/driver/core/cascaded_device.cc \
../../../../edi-framework/source/driver/core/device_info.cc \
../../../../edi-framework/source/driver/core/event_dispatcher.cpp \
../../../../edi-framework/source/driver/core/event_queue.cpp \
../../../../edi-framework/source/driver/core/factory.cc \
../../../../edi-framework/source/driver/core/general/event_forwarding.cpp \
../../../../edi-framework/source/driver/core/log_backend/android.cc \
../../../../edi-framework/source/driver/core/log.cpp \
../../../../edi-framework/source/driver/core/named_mutex.cpp \
../../../../edi-framework/source/driver/core/scheduled_execution.cpp \
../../../../edi-framework/source/driver/core/streaming_thread_common.cpp \
../../../../edi-framework/source/driver/core/support/cascaded_device_support.cpp \
../../../../edi-framework/source/driver/core/timestamp.cpp \
../../../../edi-framework/source/driver/core/v2/device_v2.cc \
../../../../edi-framework/source/driver/core/v2/eeg_session.cc \
../../../../edi-framework/source/driver/core/v2/session.cc \
../../../../edi-framework/source/driver/core/v2/session_storage.cc \
../../../../edi-framework/source/driver/core/v2/v1_wrapper.cc \
../../../../edi-framework/source/driver/usb/factory.cpp \
../../../../edi-framework/source/driver/usb/libusb/device.common.cc \
../../../../edi-framework/source/driver/usb/libusb/endpoints/bulk.cc \
../../../../edi-framework/source/driver/usb/libusb/endpoints/interrupt.cc \
../../../../edi-framework/source/driver/usb/libusb/endpoints/isochronous.cc \
../../../../edi-framework/source/driver/usb/libusb/endpoints/isochronous_transfer.cc \
../../../../edi-framework/source/driver/usb/libusb/guard.cc \
../../../../edi-framework/source/driver/usb/libusb/manager.android.cc \
../../../../edi-framework/source/driver/usb/libusb/repository.cc \
../../../../edi-framework/source/edi/DataBuffer.cpp \
../../../../edi-framework/source/edi/Data.cpp \
../../../../edi-framework/source/edi/Impedance.cpp
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES       := ../edi-framework/include ../edi-framework/source ../edi-framework/source/driver/util ../edi-framework/3rdParty/boost_android
LOCAL_CFLAGS           := -DEDI_HAVE_ANDROID -std=c++14
LOCAL_MODULE           := edi-util
LOCAL_SRC_FILES        := \
../../../../edi-framework/source/driver/util/system.android.cc
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES       := ../edi-framework/include ../edi-framework/source ../edi-framework/include/shared ../edi-framework/3rdParty/boost_android ../edi-framework/3rdParty/rtl_tcp_andro-/jni/libusb-andro
LOCAL_CFLAGS           := -DEDI_HAVE_ANDROID -DEDI_HAVE_LIBUSB -std=c++14
LOCAL_MODULE           := edi-eego
LOCAL_SRC_FILES        := \
../../../../edi-framework/source/driver/eego/command.cc \
../../../../edi-framework/source/driver/eego/commands/auxusb.cc \
../../../../edi-framework/source/driver/eego/commands/eego.cc \
../../../../edi-framework/source/driver/eego/commands/eegomini.cc \
../../../../edi-framework/source/driver/eego/config.cc \
../../../../edi-framework/source/driver/eego/configs/cfg_auxusb.cc \
../../../../edi-framework/source/driver/eego/configs/cfg_eego.cc \
../../../../edi-framework/source/driver/eego/configs/cfg_eegomini.cc \
../../../../edi-framework/source/driver/eego/device.cpp \
../../../../edi-framework/source/driver/eego/device_factory.cpp \
../../../../edi-framework/source/driver/eego/factory_access.cpp \
../../../../edi-framework/source/driver/eego/manager.cpp \
../../../../edi-framework/source/driver/eego/manager.linux.cpp \
../../../../edi-framework/source/driver/eego/parser.cc \
../../../../edi-framework/source/driver/eego/parsers/auxusb_battery_parser.cc \
../../../../edi-framework/source/driver/eego/parsers/auxusb_data_parser.cc \
../../../../edi-framework/source/driver/eego/parsers/auxusb_message_parser.cc \
../../../../edi-framework/source/driver/eego/parsers/channel_extraction_helper.cc \
../../../../edi-framework/source/driver/eego/parsers/eego_battery_parser.cc \
../../../../edi-framework/source/driver/eego/parsers/eego_data_parser.cc \
../../../../edi-framework/source/driver/eego/parsers/eego_impedance/average_calculator.cc \
../../../../edi-framework/source/driver/eego/parsers/eego_impedance/calculator.cc \
../../../../edi-framework/source/driver/eego/parsers/eego_impedance/channel_impedance.cc \
../../../../edi-framework/source/driver/eego/parsers/eego_impedance/flank_detector.cc \
../../../../edi-framework/source/driver/eego/parsers/eego_impedance_parser.cc \
../../../../edi-framework/source/driver/eego/parsers/eego_impedance_parser_v2.cc \
../../../../edi-framework/source/driver/eego/parsers/eego_impedance/peak_calculator.cc \
../../../../edi-framework/source/driver/eego/parsers/eego_impedance/reference_impedance.cc \
../../../../edi-framework/source/driver/eego/parsers/eego_impedance_v2/average_point_extractor.cpp \
../../../../edi-framework/source/driver/eego/parsers/eego_impedance_v2/base_impedance_v2.cc \
../../../../edi-framework/source/driver/eego/parsers/eego_impedance_v2/channel_impedance_v2.cc \
../../../../edi-framework/source/driver/eego/parsers/eego_impedance_v2/data_point_store.cpp \
../../../../edi-framework/source/driver/eego/parsers/eego_impedance_v2/interpolatation_differ.cpp \
../../../../edi-framework/source/driver/eego/parsers/eego_impedance_v2/reference_impedance_v2.cc \
../../../../edi-framework/source/driver/eego/parsers/eego_message_parser.cc \
../../../../edi-framework/source/driver/eego/parsers/eegomini_battery_parser.cc \
../../../../edi-framework/source/driver/eego/parsers/eegomini_data_parser.cc \
../../../../edi-framework/source/driver/eego/parsers/eegomini_impedance_parser.cc \
../../../../edi-framework/source/driver/eego/parsers/eegomini_message_parser.cc \
../../../../edi-framework/source/driver/eego/parsers/eego_trigger_parser.cc \
../../../../edi-framework/source/driver/eego/parsers/frequency_detector/frequency_detector_channel.cpp \
../../../../edi-framework/source/driver/eego/parsers/frequency_detector/frequency_detector.cpp \
../../../../edi-framework/source/driver/eego/parsers/impedance_frequency_parser.cc \
../../../../edi-framework/source/driver/eego/parsers/impedance_helpers.cpp \
../../../../edi-framework/source/driver/eego/parsers/impedance_settings.cpp \
../../../../edi-framework/source/driver/eego/streaming_thread.cpp
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES       := ../edi-framework/include ../edi-framework/source ../edi-framework/sdk ../edi-framework/3rdParty/boost_android
LOCAL_CFLAGS           := -DEDI_HAVE_ANDROID -D_snprintf=snprintf -std=c++14
LOCAL_MODULE           := eego-SDK
LOCAL_STATIC_LIBRARIES := edi-eego edi-core edi-util boost-system
LOCAL_LDLIBS           := -llog
LOCAL_SRC_FILES        := \
../../../../edi-framework/sdk/sdk-manager-singleton.cc \
../../../../edi-framework/sdk/sdk-private.cc \
../../../../edi-framework/sdk/sdk-stream.cc \
../../../../edi-framework/sdk/sdk-utils.common.cc \
../../../../edi-framework/sdk/sdk-utils.unix.cc
include $(BUILD_SHARED_LIBRARY)

