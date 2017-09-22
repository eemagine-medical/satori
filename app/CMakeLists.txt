cmake_minimum_required(VERSION 3.4.1)

if(EXISTS ${CMAKE_SOURCE_DIR}/user.cmake)
   include(${CMAKE_SOURCE_DIR}/user.cmake)
endif()

set(CMAKE_CXX_STANDARD 14)

if(NOT EXISTS ${SDK_ROOT})
  message(FATAL_ERROR "define SDK_ROOT in user.cmake")
endif()

set(CMAKE_C_FLAGS "-Wno-error=format-security")

find_library(log-lib log)

add_library(eego-SDK SHARED IMPORTED)
set_target_properties(eego-SDK PROPERTIES IMPORTED_LOCATION ${SDK_ROOT}/android/${ANDROID_ABI}/libeego-SDK.so)

add_library(app-glue STATIC
  ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

add_library(satori SHARED
  ${SDK_ROOT}/eemagine/sdk/wrapper.cc
  src/main/cpp/sdk-test1.cc
  src/main/cpp/sdk-helper.cc
  src/main/cpp/gl-helper.cc
)
target_compile_definitions(satori PRIVATE EEGO_SDK_BIND_STATIC _UNICODE)
target_include_directories(satori PRIVATE src/main/cpp)
target_include_directories(satori PRIVATE SYSTEM ${SDK_ROOT})
target_include_directories(satori PRIVATE SYSTEM ${BOOST_SOURCES})
target_include_directories(satori PRIVATE SYSTEM ${ANDROID_NDK}/sources/android/native_app_glue)

target_link_libraries(satori eego-SDK app-glue ${log-lib} GLESv2 android EGL)

add_custom_command(TARGET satori POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${SDK_ROOT}/android/${ANDROID_ABI}/libeego-SDK.so
  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libeego-SDK.so
)
