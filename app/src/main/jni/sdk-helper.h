#pragma once

// android
#include <android/log.h>

///////////////////////////////////////////////////////////////////////////////
#if _DEBUG
#define SDK_MARKER_C_STR(msg) { __android_log_print(ANDROID_LOG_DEBUG, "eego", "%s(%i/%s): %s", __FILE__, __LINE__, __FUNCTION__, msg); }
#define SDK_LOG_STREAM(stream) { std::ostringstream o; o << stream; SDK_MARKER_C_STR(o.str().c_str()); }
#else
#define SDK_MARKER_C_STR(msg)
#define SDK_LOG_STREAM(stream)
#endif

void sdk_helper_init(const char *);
void sdk_helper_fini();

struct minmax {
  float min;
  float max;
  float avg;
};

struct vertex {
    float x;
    float y;
};

struct summary {
    double alpha;
    double theta;
    bool session;
};

size_t sdk_helper_get_sample_count();
size_t sdk_helper_get_channel_count();

vertex * sdk_helper_get_bandpower_data(size_t);
vertex * sdk_helper_get_channel_data(size_t);
      float   sdk_helper_get_channel_offset(size_t);
//    float   sdk_helper_get_channel_scale(size_t);
//    float   sdk_helper_get_channel_dc(size_t);
     minmax   sdk_helper_get_channel_minmax();
minmax   sdk_helper_get_bandpower_minmax();

summary sdk_helper_get_session_summary();

void sdk_helper_fetch();
void sdk_helper_simulate();
