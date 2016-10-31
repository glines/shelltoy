#ifndef SHELLTOY_FONTS_H_
#define SHELLTOY_FONTS_H_

#include <ft2build.h>
#include FT_FREETYPE_H

void st_initFreetype();
void st_loadFontFace();
void st_loadGlyph(char c);
void st_printGlyphDebug(const FT_Bitmap *bitmap);

#endif
