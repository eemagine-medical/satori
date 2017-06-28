#include "gl-helper.h"
///////////////////////////////////////////////////////////////////////////////
GLuint gl_helper_load_shader(GLenum type, const char *shaderSrc) {
  GLuint shader;
  GLint compiled;

// Create the shader object
  shader = glCreateShader(type);
  if(shader == 0)
    return 0;
// Load the shader source
  glShaderSource(shader, 1, &shaderSrc, NULL);

// Compile the shader
  glCompileShader(shader);
// Check the compile status
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if(!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
#if 0
    if(infoLen > 1) {
      char* infoLog = malloc(sizeof(char) * infoLen);
      glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
      esLogMessage("Error compiling shader:\n%s\n", infoLog);
      free(infoLog);
    }
#endif
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}
///////////////////////////////////////////////////////////////////////////////
GLuint gl_helper_load_program(const char * vertex_shader_string, const char * fragment_shader_string) {
  auto vertexShader(gl_helper_load_shader(GL_VERTEX_SHADER, vertex_shader_string));
  auto fragmentShader(gl_helper_load_shader(GL_FRAGMENT_SHADER, fragment_shader_string));

  auto programObject(glCreateProgram());
  if(programObject == 0) {
    return -1;
  }

  glAttachShader(programObject, vertexShader);
  glAttachShader(programObject, fragmentShader);

  glBindAttribLocation(programObject, 0, "vPosition");

  glLinkProgram(programObject);

  return programObject;
}

