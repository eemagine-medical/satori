#include <jni.h>
#include <errno.h>
#include <math.h>
#include <chrono>
#include <mutex>
#include <regex>
#include <stdexcept>
#include <thread>
// gl
#include <EGL/egl.h>
// android
#include <android/asset_manager.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/sensor.h>
#include <android/window.h>
// self
#include "gl-helper.h"
#include "sdk-helper.h"



///////////////////////////////////////////////////////////////////////////////
class engine {

public:
  engine() : app(NULL), display(EGL_NO_DISPLAY), context(EGL_NO_CONTEXT), surface(EGL_NO_SURFACE), scale(5) {
    SDK_MARKER_C_STR(__PRETTY_FUNCTION__);
  }

  struct android_app* app;

  // sensors
  ASensorManager    * sensorManager;
  const ASensor     * accelerometerSensor;
  const ASensor     * magneticSensor;
  const ASensor     * gyroscopeSensor;
  ASensorEventQueue * sensorEventQueue;

  // display
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;

  int32_t width;
  int32_t height;

  // gles2 stuff
  std::vector<GLuint> gl_programs;
	std::vector<GLuint>  gl_program_u_offsets;
	std::vector<GLuint>  gl_program_u_scales;
	std::vector<GLuint>  gl_program_u_dcs;

  // settings
  float scale;
};
///////////////////////////////////////////////////////////////////////////////
/**
 * Initialize an EGL context for the current display.
 * TODO tidy this up, currently it's mostly Google example code
 */
int init_display(struct engine* engine)
{
  SDK_MARKER_C_STR(__PRETTY_FUNCTION__);

  // Setup OpenGL ES 2
  // http://stackoverflow.com/questions/11478957/how-do-i-create-an-opengl-es-2-context-in-a-native-activity

  const EGLint attribs[] = {
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, //important
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_DEPTH_SIZE, 24,
    EGL_NONE
  };

  EGLint attribList[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
  };

  EGLint w, h, dummy, visual_id;
  EGLint numConfigs;
  EGLConfig config;
  EGLSurface surface;
  EGLContext context;

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  eglInitialize(display, 0, 0);

  /* Here, the application chooses the configuration it desires. In this
   * sample, we have a very simplified selection process, where we pick
   * the first EGLConfig that matches our criteria */
  eglChooseConfig(display, attribs, &config, 1, &numConfigs);

  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
   * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
   * As soon as we picked a EGLConfig, we can safely reconfigure the
   * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &visual_id);
  ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, visual_id);
  ANativeActivity_setWindowFlags(engine->app->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);

  surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);

  context = eglCreateContext(display, config, NULL, attribList);

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {

    return -1;
  }

  // Grab the width and height of the surface
/*
  eglQuerySurface(display, surface, EGL_WIDTH, &w);
  eglQuerySurface(display, surface, EGL_HEIGHT, &h);
*/

  engine->display = display;
  engine->context = context;
  engine->surface = surface;
  engine->width = 0;
  engine->height = 0;

  // setup rendering stuff
  const char* vertex_shader_string = R"foo(
attribute vec4 vPosition;
uniform float offset;
uniform float scale;
uniform float dc;
void main() {
gl_Position = vec4(vPosition.x, offset + (vPosition.y - dc) * scale, 0, 1);
}
)foo";
  const char* fragment_shader_string_orange = R"foo(
precision mediump float;
void main() {
gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0);
}
)foo";
	const char* fragment_shader_string_cyan = R"foo(
precision mediump float;
void main() {
gl_FragColor = vec4(0.0, 0.8, 1.0, 1.0);
}
)foo";
	const char* fragment_shader_string_white = R"foo(
precision mediump float;
void main() {
gl_FragColor = vec4(0.8, 0.8, 0.8, 1.0);
}
)foo";

	engine->gl_programs.push_back(gl_helper_load_program(vertex_shader_string, fragment_shader_string_cyan));
	engine->gl_program_u_offsets.push_back(glGetUniformLocation(engine->gl_programs[0], "offset"));
	engine->gl_program_u_scales.push_back(glGetUniformLocation(engine->gl_programs[0], "scale"));
	engine->gl_program_u_dcs.push_back(glGetUniformLocation(engine->gl_programs[0], "dc"));
	engine->gl_programs.push_back(gl_helper_load_program(vertex_shader_string, fragment_shader_string_orange));
	engine->gl_program_u_offsets.push_back(glGetUniformLocation(engine->gl_programs[1], "offset"));
	engine->gl_program_u_scales.push_back(glGetUniformLocation(engine->gl_programs[1], "scale"));
	engine->gl_program_u_dcs.push_back(glGetUniformLocation(engine->gl_programs[1], "dc"));
	engine->gl_programs.push_back(gl_helper_load_program(vertex_shader_string, fragment_shader_string_white));
	engine->gl_program_u_offsets.push_back(glGetUniformLocation(engine->gl_programs[1], "offset"));
	engine->gl_program_u_scales.push_back(glGetUniformLocation(engine->gl_programs[1], "scale"));
	engine->gl_program_u_dcs.push_back(glGetUniformLocation(engine->gl_programs[1], "dc"));
	return 0;
}

///////////////////////////////////////////////////////////////////////////////


void renderTraces(struct engine * engine, bool doScaling, bool showBandPower=true)
{
	// if in band power mode, display band power traces
	if (showBandPower) {
		// scale curves
		auto minmax(sdk_helper_get_bandpower_minmax());
		auto delta(minmax.max - minmax.min);

		for (int c = 0; c < 2; ++c) {
			glUseProgram(engine->gl_programs[c]);
			glUniform1f(engine->gl_program_u_offsets[c], 0);
			if (doScaling) {
				glUniform1f(engine->gl_program_u_scales[c], 2.0);
				glUniform1f(engine->gl_program_u_dcs[c], 0.5);
			}
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, sdk_helper_get_bandpower_data(c));
			glEnableVertexAttribArray(0);
			glDrawArrays(GL_LINE_STRIP, 0, sdk_helper_get_sample_count());
		}
	}

	// else display channel traces
	else {
		// scale curves
		auto minmax(sdk_helper_get_channel_minmax());
		auto delta(minmax.max - minmax.min);

		for(int c=0; c < sdk_helper_get_channel_count(); ++c) {
			glUseProgram(engine->gl_programs[2]);
			glUniform1f(engine->gl_program_u_offsets[2],
						(float) 0.95 * sdk_helper_get_channel_offset(c));
			if (doScaling) {
				glUniform1f(engine->gl_program_u_scales[2], engine->scale * 0.04 / delta);
				glUniform1f(engine->gl_program_u_dcs[2], minmax.avg);
			}
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, sdk_helper_get_channel_data(c));
			glEnableVertexAttribArray(0);
			glDrawArrays(GL_LINE_STRIP, 0, sdk_helper_get_sample_count());
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void draw_frame(struct engine* engine)
{
	if (engine->display == NULL) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		return;
	}
	bool bandPowerMode = true;
	{
		EGLint width;
		EGLint height;
		eglQuerySurface(engine->display, engine->surface, EGL_WIDTH, &width);
		eglQuerySurface(engine->display, engine->surface, EGL_HEIGHT, &height);
		if(width != engine->width || height != engine->height) {
			glViewport(0,0,width,height);
			engine->width = width;
			engine->height = height;
			__android_log_print(ANDROID_LOG_DEBUG, "satori", "resize(%i, %i)", width, height);
		}
		bandPowerMode = (width > height);
	}

	sdk_helper_fetch();
	//sdk_helper_simulate();

	summary s = sdk_helper_get_session_summary();
	if(s.session) {
		// dark red (0.6, 0, 0) for alpha=1, theta=0
		// pale blue (0.4, 0.4, 1.0) for alpha=0, theta=0
		glClearColor(0.6*s.alpha + 0.4*s.theta, 0.4*s.theta, s.theta, 0.8);
	} else {
		glClearColor(0.2, 0.2, 0.2, 0.8);
	}
	glClear(GL_COLOR_BUFFER_BIT);

	renderTraces(engine, true, bandPowerMode);

	eglSwapBuffers(engine->display, engine->surface);
}

///////////////////////////////////////////////////////////////////////////////

void terminate_display(struct engine* engine)
{
  if (engine->display != EGL_NO_DISPLAY) {
    eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (engine->context != EGL_NO_CONTEXT) {
      eglDestroyContext(engine->display, engine->context);
    }
    if (engine->surface != EGL_NO_SURFACE) {
      eglDestroySurface(engine->display, engine->surface);
    }
    eglTerminate(engine->display);
  }
  engine->display = EGL_NO_DISPLAY;
  engine->context = EGL_NO_CONTEXT;
  engine->surface = EGL_NO_SURFACE;
  ANativeActivity_setWindowFlags(engine->app->activity, 0, AWINDOW_FLAG_KEEP_SCREEN_ON);
}
///////////////////////////////////////////////////////////////////////////////
int32_t handle_input(struct android_app* app, AInputEvent* event) {
  struct engine* engine = (struct engine*)app->userData;

// std::lock_guard<std::mutex> lg(engine->mutex);

  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    for(int32_t i=0; i<AMotionEvent_getPointerCount(event); ++i) {
    }
    return 1;
  }

  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
    auto  key_val(AKeyEvent_getKeyCode(event));

    switch(key_val) {
      case AKEYCODE_VOLUME_UP:
        engine->scale += 1;
        return 1;
        break;
      case AKEYCODE_VOLUME_DOWN:
        engine->scale -= 1;
        if(engine->scale < 1) {
          engine->scale = 1;
        }
        return 1;
        break;
    }
  }

  return 0;
}
///////////////////////////////////////////////////////////////////////////////
void handle_cmd(struct android_app* app, int32_t cmd) {
  SDK_MARKER_C_STR(__PRETTY_FUNCTION__);
  struct engine* engine = (struct engine*)app->userData;
  switch (cmd) {
  case APP_CMD_SAVE_STATE:
    SDK_MARKER_C_STR("APP_CMD_SAVE_STATE");
    sdk_helper_fini();
    break;
  case APP_CMD_INIT_WINDOW:
    SDK_MARKER_C_STR("APP_CMD_INIT_WINDOW");
    if (engine->app->window != NULL) {
      init_display(engine);
    }
    {
      ANativeActivity* nativeActivity = engine->app->activity;                              
      const char* externalPath = nativeActivity->externalDataPath;
      SDK_MARKER_C_STR("externalPath:");
      SDK_MARKER_C_STR(externalPath);
      sdk_helper_init(nativeActivity->externalDataPath);
    }
    break;
  case APP_CMD_GAINED_FOCUS:
    SDK_MARKER_C_STR("APP_CMD_GAINED_FOCUS");
    // When our app gains focus, we start monitoring the sensors.
    if(engine->accelerometerSensor != NULL) {
      ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
      ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->accelerometerSensor, (1000L/5)*1000);
    }
    if(engine->magneticSensor != NULL) {
      ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->magneticSensor);
      ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->magneticSensor, (1000L/5)*1000);
    }
    if(engine->gyroscopeSensor != NULL) {
      ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->gyroscopeSensor);
      ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->gyroscopeSensor, (1000L/5)*1000);
    }
    break;
  case APP_CMD_TERM_WINDOW:
    SDK_MARKER_C_STR("APP_CMD_TERM_WINDOW");
    // The window is being hidden or closed, clean it up.
    terminate_display(engine);
    break;
  case APP_CMD_LOST_FOCUS:
    SDK_MARKER_C_STR("APP_CMD_LOST_FOCUS");
    // sdk_helper_fini();
    if (engine->accelerometerSensor != NULL) {
      ASensorEventQueue_disableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
    }
    if (engine->magneticSensor != NULL) {
      ASensorEventQueue_disableSensor(engine->sensorEventQueue, engine->magneticSensor);
    }
    if (engine->gyroscopeSensor != NULL) {
      ASensorEventQueue_disableSensor(engine->sensorEventQueue, engine->gyroscopeSensor);
    }
    break;
  case APP_CMD_CONFIG_CHANGED:
    SDK_MARKER_C_STR("APP_CMD_CONFIG_CHANGED");
    break;
  case APP_CMD_LOW_MEMORY:
    SDK_MARKER_C_STR("APP_CMD_LOW_MEMORY");
    break;
  case APP_CMD_START:
    SDK_MARKER_C_STR("APP_CMD_START");
    break;
  case APP_CMD_RESUME:
    SDK_MARKER_C_STR("APP_CMD_RESUME");
    break;
  case APP_CMD_PAUSE:
    SDK_MARKER_C_STR("APP_CMD_PAUSE");
    break;
  case APP_CMD_STOP:
    SDK_MARKER_C_STR("APP_CMD_STOP");
    break;
  case APP_CMD_DESTROY:
    SDK_MARKER_C_STR("APP_CMD_DESTROY");
    break;
  default:
    SDK_MARKER_C_STR("default");
    break;
  }
}
///////////////////////////////////////////////////////////////////////////////
void android_main(struct android_app* state) {
  app_dummy();

  engine engine;

  state->userData = &engine;
  state->onAppCmd = handle_cmd;
  state->onInputEvent = handle_input;
  engine.app = state;

// Prepare to monitor accelerometer
  engine.sensorManager = ASensorManager_getInstance();
  engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
  engine.magneticSensor = ASensorManager_getDefaultSensor(engine.sensorManager, ASENSOR_TYPE_MAGNETIC_FIELD);
  engine.gyroscopeSensor = ASensorManager_getDefaultSensor(engine.sensorManager, ASENSOR_TYPE_GYROSCOPE);
  engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);

// Read all pending events.
  bool run(true);
  while (run) {
    try {
      int ident;
      int events;
      struct android_poll_source* source;

      while ((ident=ALooper_pollAll(0, NULL, &events,(void**)&source)) >= 0) {

// Process this event.
        if (source != NULL) {
          source->process(state, source);
        }

// If a sensor has data, process it now.
        if (ident == LOOPER_ID_USER) {
// accel
          if (engine.accelerometerSensor != NULL) {
            ASensorEvent event;
            while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &event, 1) > 0) {
            }
          }
        }


// Check if we are exiting.
        if (state->destroyRequested != 0) {
          terminate_display(&engine);
          return;
        }
      }

// Draw the current frame
      draw_frame(&engine);
    } catch(const std::exception &e) {
      SDK_MARKER_C_STR(e.what());
      run=false;
    } catch(...) {
      SDK_MARKER_C_STR("unknown error in main");
      run=false;
    }
  }

  throw(std::runtime_error("error in main"));
}
