#include "shader.h"
#include "shaders.h"

#define SHADER_DIR assets_shaders

#define CAT(a, b) a ## b

#define ST_DEFINE_SHADER_BASE(shader, dir) \
  GLuint st_Shaders_ ## shader ## Shader() { \
    static st_Shader instance; \
    if (instance.program == 0) { \
      /* This shader has not been initialized (or it might have encountered
       * an error) */ \
      st_Shader_init( \
          &instance, \
          (const char *)CAT(dir, _ ## shader ## _vert), \
          CAT(dir, _ ## shader ## _vert_len), \
          (const char *)CAT(dir, _ ## shader ## _frag), \
          CAT(dir, _ ## shader ## _frag_len) \
          ); \
    } \
    return instance.program; \
  }
#define ST_DEFINE_SHADER(shader) ST_DEFINE_SHADER_BASE(shader, SHADER_DIR)

#include "assets_shaders_glyph.vert.c"
#include "assets_shaders_glyph.frag.c"
ST_DEFINE_SHADER(glyph)
#include "assets_shaders_background.vert.c"
#include "assets_shaders_background.frag.c"
ST_DEFINE_SHADER(background)
