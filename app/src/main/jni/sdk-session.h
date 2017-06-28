#pragma once

// system
#include <memory>
// eemagine SDK
#include <eemagine/sdk/factory.h>
// self
#include "sdk-helper.h"
#ifdef EDI_ANDROID_DO_RECORDING
#include "com.eemagine.satori-recording.h"
#endif

struct sdk_session {
  std::shared_ptr<eemagine::sdk::amplifier> amplifier = nullptr;
  std::shared_ptr<eemagine::sdk::stream>    stream = nullptr;
#ifdef EDI_ANDROID_DO_RECORDING
  std::shared_ptr<sdk_recording>            recording = nullptr;
#endif

  sdk_session(const std::shared_ptr<eemagine::sdk::amplifier> & a, const std::string & data_path, int rate) {
    if(a != nullptr) {

      const auto ref_ranges(a->getReferenceRangesAvailable());
      const auto bip_ranges(a->getBipolarRangesAvailable());
      stream = std::shared_ptr<eemagine::sdk::stream>(a->OpenEegStream(rate, ref_ranges.front(), bip_ranges.front(), 0xff, 0));
      if(stream == nullptr) {
        throw(std::runtime_error("no stream"));
      }
#ifdef EDI_ANDROID_DO_RECORDING
      recording = std::make_shared<sdk_recording>(
        data_path,
        stream,
        rate,
        a->getType(),
        a->getSerialNumber()
      );
#endif
    }
    amplifier = a;
  }

  bool is_valid() const {
    return amplifier != nullptr;
  }
};
