#include <assert.h>
#include <fontconfig/fontconfig.h>
#include <stdlib.h>
#include <string.h>

#include "fonts.h"
#include "logging.h"

#include "profile.h"

/**
 * Structure that describes a font as it is known by the Shelltoy profile. This
 * structure includes information (obtained via Fontconfig) about which
 * character codes the font supports.
 */
struct st_Profile_Internal_ {
  st_FontRefArray fonts, boldFonts;
  st_BackgroundToy *backgroundToy;
  st_TextToy *textToy;
  int dpi[2];
};

void st_Profile_init(
    st_Profile *self,
    const char *name)
{
  self->fontSize = 0.0f;
  /* Allocate memory for internal structures */
  self->internal = (st_Profile_Internal *)malloc(sizeof(st_Profile_Internal));
  st_FontRefArray_init(&self->internal->fonts);
  st_FontRefArray_init(&self->internal->boldFonts);
  self->internal->backgroundToy = NULL;
  self->internal->textToy = NULL;
  /* TODO: Allow the user to set the DPI */
  /* TODO; Automatically detect the DPI somehow? */
  self->internal->dpi[0] = 144;
  self->internal->dpi[1] = 144;
  /* Copy the name string */
  self->name = (char *)malloc(strlen(name) + 1);
  strcpy(self->name, name);
}

void st_Profile_destroy(
    st_Profile *self)
{
  /* Release references to held fonts */
#define DEC_FONT_REFS(ARRAY) \
  for (size_t i = 0; \
      i < st_FontRefArray_size(&self->internal->ARRAY); \
      ++i) \
  { \
    st_FontRef *fontRef; \
    fontRef = st_FontRefArray_get(&self->internal->ARRAY, i); \
    st_FontRef_decrement(fontRef); \
  }
  DEC_FONT_REFS(fonts)
  DEC_FONT_REFS(boldFonts)
  /* Destroy our font arrays */
  st_FontRefArray_destroy(&self->internal->fonts);
  st_FontRefArray_destroy(&self->internal->boldFonts);
  /* Free memory from internal structures */
  /* NOTE: Ownership of the toys is held by the st_Config object */
  free(self->internal);
}

void st_Profile_setFlags(
    st_Profile *self,
    uint32_t flags)
{
  /* TODO: We might need to do some processing when some of these flags change.
   * For now, there is nothing to do here. */
  self->flags = flags;
}


void
st_Profile_clearFonts(
    st_Profile *self)
{
  /* Release our held font references */
  DEC_FONT_REFS(fonts)
  DEC_FONT_REFS(boldFonts)
  /* Clear our font arrays */
  st_FontRefArray_clear(&self->internal->fonts);
  st_FontRefArray_clear(&self->internal->boldFonts);
}

st_Font *
st_Profile_getPrimaryFont(
    st_Profile *self)
{
  if (st_FontRefArray_size(&self->internal->fonts) <= 0)
    return NULL;
  return st_FontRef_get(st_FontRefArray_get(&self->internal->fonts, 0));
}

st_ErrorCode
st_Profile_setPrimaryFont(
    st_Profile *self,
    const char *fontFace,
    float fontSize)
{
  FcConfig *fc;
  FcPattern *pattern;
  FcFontSet *sourceFontSets[2];
  FcFontSet *resultFontSet;
  FcPattern *matchingFont;
  FcResult fcResult;
  FcValue fcValue;
  const char *fontPath;
  st_FontRef *fontRef;
  st_ErrorCode error;

  /* Use fontconfig to look for a font with this face and font size */
  fc = st_Fonts_getFontconfigInstance();

  /* Build a Fontconfig pattern describing our font criteria */
  pattern = FcPatternBuild(
      NULL,  /* pattern */
      FC_SPACING, FcTypeInteger, FC_MONO,  /* Look for monospace fonts... */
      FC_FULLNAME, FcTypeString, fontFace,  /* ...with this font face. */
//      FC_PIXEL_SIZE, FcTypeDouble, fontSize,  /* ...with this pixel size... */
//      FC_WEIGHT,  FcTypeInteger, FC_WEIGHT_REGULAR,  /* ...and this weight. */
      (char *) NULL  /* terminator */
      );
  if (pattern == NULL) {
    error = ST_ERROR_FONTCONFIG_ERROR;
    goto setPrimaryFont_cleanup1;
  }

  /* Look for fonts matching our criteria */
  sourceFontSets[0] = FcConfigGetFonts(fc, FcSetSystem);
  sourceFontSets[1] = FcConfigGetFonts(fc, FcSetApplication);
  resultFontSet = FcFontSetList(
      fc,  /* config */
      sourceFontSets,  /* sets */
      2,  /* nsets */
      pattern,  /* pattern */
      /* FIXME: Fontconfig documentation is not clear on the utility of
       * object_set */
      NULL  /* object_set */
      );
  if (resultFontSet == NULL) {
    error = ST_ERROR_FONTCONFIG_ERROR;
    goto setPrimaryFont_cleanup2;
  }

  /* XXX: Print out all of the matching fonts */
  for (int i = 0; i < resultFontSet->nfont; ++i) {
    FcPattern *font;
    font = resultFontSet->fonts[i];
    FcPatternPrint(font);
  }

  /* Select the first font matching our criteria */
  if (resultFontSet->nfont > 0) {
    matchingFont = resultFontSet->fonts[0];
  } else {
    ST_LOG_ERROR(
        "Fontconfig could not find any suitable fonts for face '%s'",
        fontFace);
    error = ST_ERROR_MISSING_FONT;
    goto setPrimaryFont_cleanup3;
  }

  /* Get the file path of the matching font */
  fcResult = FcPatternGet(
      matchingFont,  /* pattern */
      FC_FILE,  /* object */
      0,  /* id */
      &fcValue  /* value */
      );
  if (fcResult != FcResultMatch) {
    error = ST_ERROR_FONTCONFIG_ERROR;
    goto setPrimaryFont_cleanup3;
  }
  fontPath = (const char *)fcValue.u.s;
  if (fontPath == NULL) {
    error = ST_ERROR_FONTCONFIG_ERROR;
    goto setPrimaryFont_cleanup3;
  }

  /* Load the font with FreeType */
  fontRef = (st_FontRef *)malloc(sizeof(st_FontRef));
  st_FontRef_init(fontRef);
  st_Font_init(st_FontRef_get(fontRef));
  error = st_Font_load(st_FontRef_get(fontRef),
      fontPath,  /* fontPath */
      fontFace,  /* faceName */
      fontSize,  /* fontSize */
      self->internal->dpi[0],  /* x_dpi */
      self->internal->dpi[1]  /* y_dpi */
      );
  if (error != ST_NO_ERROR) {
    st_FontRef_decrement(fontRef);
    goto setPrimaryFont_cleanup3;
  }

  /* TODO: Clear the list of fonts if we have not done so already */
  /* The primary font is always the first font in our array of fonts. The
   * primary font determines the font size of all subsequently added fallback
   * fonts, so setting the primary font implies clearing all existing fonts. */
  st_Profile_clearFonts(self);

  /* Add the font to the beginning of our array of fonts */
  st_FontRefArray_append(&self->internal->fonts, fontRef);

  /* TODO: Look for the corresponding bold font? */

setPrimaryFont_cleanup3:
  FcFontSetDestroy(resultFontSet);
setPrimaryFont_cleanup2:
  FcPatternDestroy(pattern);
setPrimaryFont_cleanup1:

  return error;
}

st_ErrorCode
st_Profile_addFallbackFont(
    st_Profile *self,
    const char *fontFace)
{
  /* TODO */
  return ST_NO_ERROR;
}

float
st_Profile_getFontSize(
    st_Profile *self)
{
  st_Font *primaryFont;

  /* The font size is determined by the font size of the primary font */
  primaryFont = st_Profile_getPrimaryFont(self);
  if (primaryFont == NULL)
    return -1;
  return st_Font_getSize(primaryFont);
}

st_ErrorCode
st_Profile_setFontSize(
    st_Profile *self,
    float fontSize)
{
  /* TODO: Re-load the primary font with the given font size */

  /* TODO: Re-load all of the fallback fonts now that the primary font has
   * changed size */

  return ST_NO_ERROR;
}

void
st_Profile_getFonts(
    st_Profile *self,
    st_FontRefArray *fonts,
    st_FontRefArray *boldFonts)
{
  /* Copy all of our fonts to the given fonts array, incrementing reference
   * counts along the way */
#define COPY_FONT_REFS(ARRAY) \
  for (size_t i = 0; \
      i < st_FontRefArray_size(&self->internal->ARRAY); \
      ++i) \
  { \
    st_FontRef *fontRef; \
    fontRef = st_FontRefArray_get(&self->internal->ARRAY, i); \
    st_FontRefArray_append(ARRAY, fontRef); \
    st_FontRef_increment(fontRef); \
  }
  COPY_FONT_REFS(fonts)
  COPY_FONT_REFS(boldFonts)
}

void st_Profile_setBackgroundToy(
    st_Profile *self,
    st_BackgroundToy *backgroundToy)
{
  self->internal->backgroundToy = backgroundToy;
}

st_BackgroundToy *
st_Profile_getBackgroundToy(
    st_Profile *self)
{
  return self->internal->backgroundToy;
}

void st_Profile_setTextToy(
    st_Profile *self,
    st_TextToy *textToy)
{
  self->internal->textToy = textToy;
}

st_TextToy *
st_Profile_getTextToy(
    st_Profile *self)
{
  return self->internal->textToy;
}
