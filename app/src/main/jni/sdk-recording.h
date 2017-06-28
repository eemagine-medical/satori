#pragma once

// system
#include <memory>
// eemagine SDK
#include <eemagine/sdk/factory.h>
// self
#include "sdk-helper.h"

struct sdk_recording {
  int        cnt_handle = -1;
  int        channel_count = 0;
  int        sample_count = 0;
  int        trigger_channel_index = -1;
  chaninfo_t channel_info_handle;
  recinfo_t  recording_info_handle;

  sdk_recording(std::string data_path, const std::shared_ptr<eemagine::sdk::stream> & stream, int sampling_rate, const std::string & model, const std::string & serial) {
    std::string hello("hello from NDK");
    if(data_path.size()) {
      time_t     now = time(0);
      char       recording_name[80];
      auto tstruct = localtime(&now);
      strftime(recording_name, sizeof(recording_name), "/rec_%Y_%m_%d__%H_%M_%S.cnt", tstruct);

      data_path += recording_name;

      for(const auto & c: stream->getChannelList()) {
        switch(c.getType()) {
          case eemagine::sdk::channel::reference:
            ++channel_count;
            break;
          case eemagine::sdk::channel::trigger:
            trigger_channel_index = c.getIndex();
            break;
        }
      }
      SDK_LOG_STREAM("trigger channel index: " << trigger_channel_index);

      // setup channel information
      channel_info_handle = libeep_create_channel_info();
      for(int i = 0; i < channel_count; ++i) {
        std::string name(boost::str(boost::format("chan%i") % i));
	    libeep_add_channel(channel_info_handle, name.c_str(), "ref", "uV");
      }

      // setup recording info
      recording_info_handle = libeep_create_recinfo();
      libeep_set_start_time(recording_info_handle, time(NULL));
      libeep_set_machine_make(recording_info_handle, "eego");
      libeep_set_machine_model(recording_info_handle, model.c_str());
      libeep_set_machine_serial_number(recording_info_handle, serial.c_str());
      libeep_set_comment(recording_info_handle, "recorded by Android Eego App");

      cnt_handle = libeep_write_cnt(data_path.c_str(), sampling_rate, channel_info_handle, 0);
    }
  }
  ~sdk_recording() {
    libeep_add_recording_info(cnt_handle, recording_info_handle);
    libeep_close(cnt_handle);
    SDK_MARKER_C_STR("recording closed");
  }
  void write(const eemagine::sdk::buffer & b) {
	float sample_buffer[channel_count];
    for(unsigned int s=0;s<b.getSampleCount();++s) {
      // copy to sample buffer
      for(unsigned int c=0;c<channel_count;++c) {
        sample_buffer[c] = (float)(b.getSample(c, s) * 1000000.0); 
      }
      libeep_add_samples(cnt_handle, sample_buffer, 1);
#ifndef _DEBUG
      // trigger
      int code(b.getSample(trigger_channel_index, s));
      if(code > 0) {
        std::string code(boost::str(boost::format("%i") % code));
        libeep_add_trigger(cnt_handle, sample_count, code.c_str());
      }
#endif
      // inc
      ++sample_count;
    }
  }
};
