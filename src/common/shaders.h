#ifndef TTOY_COMMON_SHADERS_H_
#define TTOY_COMMON_SHADERS_H_

#include <GL/glew.h>

#define TTOY_DECLARE_SHADER(name) \
  GLuint ttoy_Shaders_ ## name ## Shader();

TTOY_DECLARE_SHADER(glyph)
TTOY_DECLARE_SHADER(background)
TTOY_DECLARE_SHADER(underline)

#endif
