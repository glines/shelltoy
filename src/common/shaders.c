#include "shaders.h"

#include "../logging.h"
#include "shader.h"

#include <stdio.h>

#define SHADER_DIR assets_shaders

#define CAT(a, b) a ## b

#define ST_DEFINE_SHADER_BASE(shader, dir) \
  GLuint st_Shaders_ ## shader ## Shader() { \
    static st_Shader instance; \
    if (instance.program == 0) { \
      st_ErrorCode error; \
      /* This shader has not been initialized (or it might have encountered
       * an error) */ \
      /* FIXME: We should call st_Shader_destroy() at some point */ \
      st_Shader_init(&instance); \
      /* Compile the vertex shader */ \
      error = st_Shader_compileShaderFromString( \
          &instance, \
          (const char *)CAT(dir, _ ## shader ## _vert), /* code */ \
          CAT(dir, _ ## shader ## _vert_len),  /* length */ \
          GL_VERTEX_SHADER  /* type */ \
          ); \
      if (error != ST_NO_ERROR) { \
        ST_LOG_ERROR( \
            "Failed to compile '%s' vertex shader", \
            #shader); \
        ST_LOG_ERROR_CODE(error); \
        st_Shader_destroy(&instance); \
        return 0; \
      } \
      /* Compile the fragment shader */ \
      error = st_Shader_compileShaderFromString( \
          &instance, \
          (const char *)CAT(dir, _ ## shader ## _frag), /* code */ \
          CAT(dir, _ ## shader ## _frag_len),  /* length */ \
          GL_FRAGMENT_SHADER  /* type */ \
          ); \
      if (error != ST_NO_ERROR) { \
        ST_LOG_ERROR( \
            "Failed to compile '%s' fragment shader", \
            #shader); \
        ST_LOG_ERROR_CODE(error); \
        st_Shader_destroy(&instance); \
        return 0; \
      } \
      /* Link the shader program */ \
      error = st_Shader_linkProgram(&instance); \
      if (error != ST_NO_ERROR) { \
        ST_LOG_ERROR( \
            "Failed to link '%s' shader program", \
            #shader); \
        ST_LOG_ERROR_CODE(error); \
        st_Shader_destroy(&instance); \
        return 0; \
      } \
    } \
    /* FIXME: We might want to check if the shader program is actually ready
     * here. It is possible for instance.program to have a value without
     * referring to a valid shader program. */ \
    return instance.program; \
  }
#define ST_DEFINE_SHADER(shader) ST_DEFINE_SHADER_BASE(shader, SHADER_DIR)

#include "assets_shaders_glyph.vert.c"
#include "assets_shaders_glyph.frag.c"
ST_DEFINE_SHADER(glyph)
#include "assets_shaders_background.vert.c"
#include "assets_shaders_background.frag.c"
ST_DEFINE_SHADER(background)
#include "assets_shaders_underline.vert.c"
#include "assets_shaders_underline.frag.c"
ST_DEFINE_SHADER(underline)
