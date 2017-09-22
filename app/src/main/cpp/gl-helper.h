#pragma once

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

GLuint gl_helper_load_shader(GLenum, const char *);
GLuint gl_helper_load_program(const char *, const char *);
