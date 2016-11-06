#ifndef ST_COMMON_SHADERS_H_
#define ST_COMMON_SHADERS_H_

#include <GL/glew.h>

#define ST_DECLARE_SHADER(name) \
  GLuint st_Shaders_ ## name ## Shader();

ST_DECLARE_SHADER(glyph)

#endif
