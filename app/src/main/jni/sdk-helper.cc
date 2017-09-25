// system
#include <sys/stat.h>
#include <sys/types.h>
// system
#include <chrono>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
// android
#include <android/log.h>
// boost
#include <boost/format.hpp>
// libeep
extern "C" {
#include <v4/eep.h>
}
// eemagine SDK
#include <eemagine/sdk/factory.h>
// self
#include "sdk-helper.h"
#ifdef EDI_ANDROID_DO_RECORDING
#include "com.eemagine.satori-recording.h"
#endif
#include "sdk-session.h"

///////////////////////////////////////////////////////////////////////////////

static std::string _app_data_path;

///////////////////////////////////////////////////////////////////////////////

class StateVariableFilter {
private:

	// filter lines
	double A,B,C,D,E,F,G,H;

	// filter parameters
	double q,f;

	// sliding window variables for turning bandpass into band power
	double mean, bandPower, window;
public:
	StateVariableFilter(double f, double q);
	bool next(double input);
	double getBandPass() {return C;};
	double getLowPass() {return F;};
	double getHighPass() {return A;};
	double getBandReject() {return A+F;};
	double getBandPower() {return bandPower;};
};

//____________________________________________________________
// Constructor

StateVariableFilter::StateVariableFilter(double f_in, double q_in=1.0)
{
	A = B = C = D = E = F = G = H = 0.0;
	q = q_in;
	f = 2.0 * sin(3.1415926535 * f_in / 512.0);
	mean = 0.0;
	bandPower = 0.0;
	window = pow(0.5, 1.0/512.0);  // window size of one second
}

//____________________________________________________________
// Calculate the next value, based on the next input

bool StateVariableFilter::next(double input)
{
	// calculate current values
	E=D*f;
	H=D*q;
	F=E+G;
	A=input-H-F;
	B=A*f;
	C=B+D;

	// z^-1 values for next run
	D=C;
	G=F;

	// sliding values for band power
	mean = mean*window + getBandPass()*(1-window);
	bandPower = bandPower*window + pow((getBandPass()-mean), 2)*(1-window);
	return true;
}


///////////////////////////////////////////////////////////////////////////////

class sdk_context {
public:
	sdk_context() : scaling(1.0)
	{
		_session = std::make_shared<sdk_session>(nullptr, "", 0);
		// 5.5 - theta, 11.0 - alpha
		_powerFilters.push_back(StateVariableFilter(8.25, 0.5)); // indicator0 - between alpha and theta, mildly broad
		_powerFilters.push_back(StateVariableFilter(20.0, 0.2)); // indicator1 - well above alpha and theta, really broad
		for (int i=0; i<8; ++i)
			_channelFilters.push_back(StateVariableFilter(15.0, 1.0));
	}

	static const size_t channel_count = 8;
	static const size_t sample_count = 1024 * 2;
	float scaling;

	// providing pand power for indicator0 and indicator1
	vertex bandPowerVertices[sample_count * 2];
	vertex channelVertices[sample_count * channel_count];
	eemagine::sdk::factory factory;

	std::shared_ptr<sdk_session> get_session()
	{
		std::shared_ptr<sdk_session> rv;
		{
		  std::lock_guard<std::mutex> lg(_mutex);
		  rv = _session;
		}
		return rv;;
	}

	void set_session(const std::shared_ptr<sdk_session> & s)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		_session = s;
	}
	static std::vector<StateVariableFilter> _powerFilters;
	static std::vector<StateVariableFilter> _channelFilters;

private:
	std::mutex                   _mutex;
	std::shared_ptr<sdk_session> _session;
};

///////////////////////////////////////////////////////////////////////////////

std::vector<StateVariableFilter> sdk_context::_powerFilters;
std::vector<StateVariableFilter> sdk_context::_channelFilters;

///////////////////////////////////////////////////////////////////////////////
static
sdk_context &
sdk_context_instance() {
  static sdk_context ctx;
  return ctx;
}

///////////////////////////////////////////////////////////////////////////////

static float _clamp(float v, float min, float max) {
  if(v<min) { return min; }
  if(v>max) { return max; }
  return v;
}

///////////////////////////////////////////////////////////////////////////////

static vertex *_getBandLinePointer(size_t band)
{
  return sdk_context_instance().bandPowerVertices + band * sdk_context::sample_count;
}

///////////////////////////////////////////////////////////////////////////////

static vertex *_getChannelLinePointer(size_t channel)
{
	return sdk_context_instance().channelVertices + channel * sdk_context::sample_count;
}

///////////////////////////////////////////////////////////////////////////////

static void _bandLineShift(size_t band, size_t count)
{
  if(count >= sdk_context::sample_count) {
    return;
  }

  auto n(sdk_context::sample_count - count);
  if(count < sdk_context::sample_count) {
    auto vertex_write(_getBandLinePointer(band));
    auto vertex_read(vertex_write + count);
    while(n--) {
      vertex_write->y = vertex_read->y;
		vertex_read->y = 0.0;
      ++vertex_write;
      ++vertex_read;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

static void _channelLineShift(size_t channel, size_t count)
{
	if(count >= sdk_context::sample_count) {
		return;
	}

	auto n(sdk_context::sample_count - count);
	if(count < sdk_context::sample_count) {
		auto vertex_write(_getChannelLinePointer(channel));
		auto vertex_read(vertex_write + count);
		while(n--) {
			vertex_write->y = vertex_read->y;
			++vertex_write;
			++vertex_read;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

static void _addToBand(size_t channel_out, const double *data, size_t data_count,
					   size_t data_stride)
{
	auto read_ptr(data);

	if(data_count > sdk_context::sample_count) {
		auto correction(sdk_context::sample_count - data_count);
		read_ptr += correction;
		data_count = sdk_context::sample_count;
	}

	auto vertex_write(_getBandLinePointer(channel_out) + sdk_context::sample_count - data_count);

	while(data_count--) {
		vertex_write->y += *read_ptr;
		++vertex_write;
		read_ptr += data_stride;
	}
}

///////////////////////////////////////////////////////////////////////////////

static void _writeToChannel(size_t channel_out, const double *data, size_t data_count,
						size_t data_stride)
{

	auto read_ptr(data);

	if(data_count > sdk_context::sample_count) {
		auto correction(sdk_context::sample_count - data_count);
		read_ptr += correction;
		data_count = sdk_context::sample_count;
	}

	auto vertex_write(_getChannelLinePointer(channel_out) + sdk_context::sample_count - data_count);

	while(data_count--) {
		vertex_write->y = (float)*read_ptr;
		++vertex_write;
		read_ptr += data_stride;
	}
}

///////////////////////////////////////////////////////////////////////////////

static void _bandLineFilter(size_t c, size_t data_count)
{
	if(data_count > sdk_context::sample_count)
		data_count = sdk_context::sample_count;

	auto vertex_filter(_getBandLinePointer(c) + sdk_context::sample_count - data_count);

	while(data_count--) {
		sdk_context::_powerFilters[c].next(vertex_filter->y);
		vertex_filter->y = (float)sdk_context::_powerFilters[c].getBandPower();
		++vertex_filter;
	}
}

///////////////////////////////////////////////////////////////////////////////

static void _channelLineFilter(size_t c, size_t data_count)
{
	if(data_count > sdk_context::sample_count)
		data_count = sdk_context::sample_count;

	auto vertex_filter(_getChannelLinePointer(c) + sdk_context::sample_count - data_count);

	while(data_count--) {
		sdk_context::_channelFilters[c].next(vertex_filter->y);
		vertex_filter->y = (float)sdk_context::_channelFilters[c].getBandPass();
		++vertex_filter;
	}
}

///////////////////////////////////////////////////////////////////////////////

static void _normaliseBands(size_t data_count)
{
	if(data_count > sdk_context::sample_count)
		data_count = sdk_context::sample_count;

	auto raw0(_getBandLinePointer(0) + sdk_context::sample_count - data_count);
	auto raw1(_getBandLinePointer(1) + sdk_context::sample_count - data_count);

	while(data_count--) {
		double sum = raw0->y + raw1->y;
		raw0->y /= sum;
		raw1->y /= sum;
		++raw0;
		++raw1;
	}
}

///////////////////////////////////////////////////////////////////////////////

void sdk_helper_init(const char * data_path)
{
	_app_data_path = data_path;
	try {

		// initialise band power data
		for(size_t c=0; c<2; ++c) {
			auto vertex_write(_getBandLinePointer(c));
			for (size_t s = 0; s < sdk_context::sample_count; ++s) {
				vertex_write->x =
					0.98f*(-1.0f + 2.0f*float(s)/(float(sdk_context::sample_count - 1)));
				vertex_write->y = float( 0.5 + 0.4*sin(0.01*double(s) + double(c)) );
				++vertex_write;
			}
		}

		// initialise channel data
		for(size_t c=0; c < sdk_context::channel_count; ++c) {
			auto vertex_write(_getChannelLinePointer(c));
			for(size_t s=0; s<sdk_context::sample_count; ++s) {
				vertex_write->x =
					0.98f * (-1.0f + 2.0f*float(s)/(float(sdk_context::sample_count - 1)));
				vertex_write->y = float(sin(double(s) * 0.01));
				++vertex_write;
			}
		}

		SDK_MARKER_C_STR(__FUNCTION__);
		auto v(sdk_context_instance().factory.getVersion());
		std::string msg(boost::str(boost::format("eemagine com.eemagine.satori version(%i, %i, %i, %i)") % v.major % v.minor % v.micro % v.build));
		SDK_MARKER_C_STR(msg.c_str());
	} catch(const std::exception &e) {
		SDK_MARKER_C_STR(e.what());
	} catch(...) {
		SDK_MARKER_C_STR(__FUNCTION__);
	}
}

///////////////////////////////////////////////////////////////////////////////

void sdk_helper_fini()
{
  try {
    SDK_MARKER_C_STR(__FUNCTION__);
    sdk_context_instance().set_session(std::make_shared<sdk_session>(nullptr, "", 0));
  } catch(const std::exception &e) {
    SDK_MARKER_C_STR(e.what());
  } catch(...) {
    SDK_MARKER_C_STR(__FUNCTION__);
  }
}

///////////////////////////////////////////////////////////////////////////////

size_t sdk_helper_get_sample_count() {
  return sdk_context::sample_count;
}

///////////////////////////////////////////////////////////////////////////////

size_t sdk_helper_get_channel_count()
{
  return sdk_context::channel_count;
}

///////////////////////////////////////////////////////////////////////////////

void sdk_helper_fetch()
{
	try {
		auto session(sdk_context_instance().get_session());
		if(session->is_valid()) {
			auto data(session->stream->getData());
			// to render buffer
			auto nSamples = data.getSampleCount();
			auto nChannels = data.getChannelCount();
			if(nSamples) {
				// to render buffer for band power displays
				for (size_t b=0; b<2; ++b) {
					_bandLineShift(b, nSamples);
					for (size_t c = 0; c < sdk_context::channel_count; ++c)
						_addToBand(b, data.data() + c, nSamples, nChannels);
					_bandLineFilter(b, nSamples);
				}
				_normaliseBands(nSamples);

				// to render buffer for channel displays
				for (size_t c=0; c < sdk_context::channel_count; ++c) {
					_channelLineShift(c, nSamples);
					_writeToChannel(c, data.data() + c, nSamples, nChannels);
					_channelLineFilter(c, nSamples);
				}
			}

#ifdef EDI_ANDROID_DO_RECORDING
      // to recording
      session->recording->write(data);
#endif
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			std::shared_ptr<eemagine::sdk::amplifier> amplifier(sdk_context_instance().factory.getAmplifier());
			sdk_context_instance().set_session(std::make_shared<sdk_session>(amplifier, _app_data_path, 512));
		}
	} catch(const eemagine::sdk::exceptions::notConnected &) {
		SDK_MARKER_C_STR("not connected");
		sdk_context_instance().set_session(std::make_shared<sdk_session>(nullptr, "", 0));
		__android_log_print(ANDROID_LOG_DEBUG, "satori", "not connected");
	} catch(const std::exception &e) {
		SDK_MARKER_C_STR(e.what());
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		__android_log_print(ANDROID_LOG_DEBUG, "satori", "not connected (%s)", e.what());
	} catch(...) {
		SDK_MARKER_C_STR(__FUNCTION__)
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		__android_log_print(ANDROID_LOG_DEBUG, "satori", "not connected (%s)", __FUNCTION__);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// use for simulating data

double _test_data[8];
void sdk_helper_simulate()
{
	auto nSamples = 1;
	auto nChannels = 8;
	for (int i=0; i<nChannels; ++i) {
		double random = double(rand() % 1000) / 1000.0 - 0.5;
		_test_data[i] = 0.9*_test_data[i] - 0.01*_test_data[i] + 0.09*random;
	}

	// to render buffer for band power displays
	for (size_t b=0; b<2; ++b) {
		_bandLineShift(b, nSamples);
		for (size_t c = 0; c < sdk_context::channel_count; ++c)
			_addToBand(b, &_test_data[0] + c, nSamples, 2);
		_bandLineFilter(b, nSamples);
	}
	_normaliseBands(nSamples);

	// to render buffer for channel displays
	for (size_t c=0; c < sdk_context::channel_count; ++c) {
		_channelLineShift(c, nSamples);
		_writeToChannel(c, &_test_data[0] + c, nSamples, nChannels);
		_channelLineFilter(c, nSamples);
	}
}

///////////////////////////////////////////////////////////////////////////////

vertex * sdk_helper_get_bandpower_data(size_t band)
{
  return & sdk_context_instance().bandPowerVertices[band * sdk_context::sample_count];
}

///////////////////////////////////////////////////////////////////////////////

vertex * sdk_helper_get_channel_data(size_t channel)
{
	return & sdk_context_instance().channelVertices[channel * sdk_context::sample_count];
}

///////////////////////////////////////////////////////////////////////////////

float sdk_helper_get_channel_offset(size_t channel)
{
  return 1.0 - 2.0 * (float)(channel + 1) / float(sdk_context::channel_count + 1);
}

///////////////////////////////////////////////////////////////////////////////

minmax sdk_helper_get_channel_minmax()
{
  bool   first(true);
  minmax rv;
  auto   rp(&sdk_context_instance().channelVertices[0]);

  rv.avg=0;
  for(size_t n=1; n < sdk_context::channel_count*sdk_context::sample_count; ++n) {
    auto y(rp->y);
    ++rp;
    if(first) {
      rv.min=y;
      rv.max=y;
		first = false;
    } else {
      if(y < rv.min) { rv.min=y; }
      if(y > rv.max) { rv.max=y; }
    }
    rv.avg += y;
  }
  rv.avg /= ((float)sdk_context::sample_count*sdk_context::channel_count);
  return rv;
}


///////////////////////////////////////////////////////////////////////////////

minmax sdk_helper_get_bandpower_minmax()
{
	bool   first(true);
	minmax rv;
	auto   rp(&sdk_context_instance().bandPowerVertices[0]);

	rv.avg=0;
	for(size_t n=1; n < 2*sdk_context::sample_count; ++n) {
		auto y(rp->y);
		++rp;
		if(first) {
			rv.min=y;
			rv.max=y;
			first = false;
		} else {
			if(y < rv.min) { rv.min=y; }
			if(y > rv.max) { rv.max=y; }
		}
		rv.avg += y;
	}
	rv.avg /= ((float)sdk_context::sample_count*2.0);
	return rv;
}

///////////////////////////////////////////////////////////////////////////////

summary sdk_helper_get_session_summary()
{
	auto session(sdk_context_instance().get_session());
	summary result;
	result.session = session->is_valid();
	double ind0 = sdk_context::_powerFilters[0].getBandPower();
	double ind1 = sdk_context::_powerFilters[1].getBandPower();
	if (ind0+ind1 > 0.0) {
		result.indicator0 = ind0 / (ind0 + ind1);
		result.indicator1 = ind1 / (ind0 + ind1);
	} else {
		result.indicator0 = 0.0;
		result.indicator1 = 0.0;
	}
	return result;
}
