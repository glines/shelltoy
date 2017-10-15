#include <assert.h>
#include <fontconfig/fontconfig.h>
#include <stdlib.h>
#include <string.h>

#include "fonts.h"
#include "logging.h"

#include "profile.h"

/* Private methods */
ttoy_ErrorCode
ttoy_Profile_adjustFallbackFontSize(
    ttoy_Profile *self,
    ttoy_Font *font);

/**
 * Structure that describes a font as it is known by the ttoy profile. This
 * structure includes information (obtained via Fontconfig) about which
 * character codes the font supports.
 */
struct ttoy_Profile_Internal_ {
  ttoy_FontRefArray fonts, boldFonts;
  ttoy_BackgroundToy *backgroundToy;
  ttoy_TextToy *textToy;
  int dpi[2];
};

void ttoy_Profile_init(
    ttoy_Profile *self,
    const char *name)
{
  self->fontSize = 0.0f;
  /* Allocate memory for internal structures */
  self->internal = (ttoy_Profile_Internal *)malloc(sizeof(ttoy_Profile_Internal));
  ttoy_FontRefArray_init(&self->internal->fonts);
  ttoy_FontRefArray_init(&self->internal->boldFonts);
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

void ttoy_Profile_destroy(
    ttoy_Profile *self)
{
  /* Release references to held fonts */
#define DEC_FONT_REFS(ARRAY) \
  for (size_t i = 0; \
      i < ttoy_FontRefArray_size(&self->internal->ARRAY); \
      ++i) \
  { \
    ttoy_FontRef *fontRef; \
    fontRef = ttoy_FontRefArray_get(&self->internal->ARRAY, i); \
    ttoy_FontRef_decrement(fontRef); \
  }
  DEC_FONT_REFS(fonts)
  DEC_FONT_REFS(boldFonts)
  /* Destroy our font arrays */
  ttoy_FontRefArray_destroy(&self->internal->fonts);
  ttoy_FontRefArray_destroy(&self->internal->boldFonts);
  /* Free memory from internal structures */
  /* NOTE: Ownership of the toys is held by the ttoy_Config object */
  free(self->internal);
}

void ttoy_Profile_setFlags(
    ttoy_Profile *self,
    uint32_t flags)
{
  /* TODO: We might need to do some processing when some of these flags change.
   * For now, there is nothing to do here. */
  self->flags = flags;
}


void
ttoy_Profile_clearFonts(
    ttoy_Profile *self)
{
  /* Release our held font references */
  DEC_FONT_REFS(fonts)
  DEC_FONT_REFS(boldFonts)
  /* Clear our font arrays */
  ttoy_FontRefArray_clear(&self->internal->fonts);
  ttoy_FontRefArray_clear(&self->internal->boldFonts);
}

ttoy_Font *
ttoy_Profile_getPrimaryFont(
    ttoy_Profile *self)
{
  if (ttoy_FontRefArray_size(&self->internal->fonts) <= 0)
    return NULL;
  return ttoy_FontRef_get(ttoy_FontRefArray_get(&self->internal->fonts, 0));
}

ttoy_ErrorCode
ttoy_Profile_setPrimaryFont(
    ttoy_Profile *self,
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
  ttoy_FontRef *fontRef;
  ttoy_ErrorCode error;

  /* Use fontconfig to look for a font with this face and font size */
  fc = ttoy_Fonts_getFontconfigInstance();

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
    error = TTOY_ERROR_FONTCONFIG_ERROR;
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
    error = TTOY_ERROR_FONTCONFIG_ERROR;
    goto setPrimaryFont_cleanup2;
  }

//  /* XXX: Print out all of the matching fonts */
//  for (int i = 0; i < resultFontSet->nfont; ++i) {
//    FcPattern *font;
//    font = resultFontSet->fonts[i];
//    FcPatternPrint(font);
//  }

  /* Select the first font matching our criteria */
  if (resultFontSet->nfont > 0) {
    matchingFont = resultFontSet->fonts[0];
  } else {
    TTOY_LOG_ERROR(
        "Fontconfig could not find any suitable fonts for face '%s'",
        fontFace);
    error = TTOY_ERROR_MISSING_FONT;
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
    error = TTOY_ERROR_FONTCONFIG_ERROR;
    goto setPrimaryFont_cleanup3;
  }
  fontPath = (const char *)fcValue.u.s;
  if (fontPath == NULL) {
    error = TTOY_ERROR_FONTCONFIG_ERROR;
    goto setPrimaryFont_cleanup3;
  }

  /* Load the font with FreeType */
  ttoy_FontRef_init(&fontRef);
  ttoy_Font_init(ttoy_FontRef_get(fontRef));
  error = ttoy_Font_load(ttoy_FontRef_get(fontRef),
      fontPath,  /* fontPath */
      fontFace,  /* faceName */
      fontSize,  /* fontSize */
      self->internal->dpi[0],  /* x_dpi */
      self->internal->dpi[1]  /* y_dpi */
      );
  if (error != TTOY_NO_ERROR) {
    ttoy_FontRef_decrement(fontRef);
    goto setPrimaryFont_cleanup3;
  }

  /* TODO: Clear the list of fonts if we have not done so already */
  /* The primary font is always the first font in our array of fonts. The
   * primary font determines the font size of all subsequently added fallback
   * fonts, so setting the primary font implies clearing all existing fonts. */
  ttoy_Profile_clearFonts(self);

  /* Add the font to the beginning of our array of fonts */
  ttoy_FontRefArray_append(&self->internal->fonts, fontRef);

  /* TODO: Look for the corresponding bold font? */

setPrimaryFont_cleanup3:
  FcFontSetDestroy(resultFontSet);
setPrimaryFont_cleanup2:
  FcPatternDestroy(pattern);
setPrimaryFont_cleanup1:

  return error;
}

ttoy_ErrorCode
ttoy_Profile_addFallbackFont(
    ttoy_Profile *self,
    const char *fontFace)
{
  /* TODO */
  return TTOY_NO_ERROR;
}

float
ttoy_Profile_getFontSize(
    ttoy_Profile *self)
{
  ttoy_Font *primaryFont;

  /* The font size is determined by the font size of the primary font */
  primaryFont = ttoy_Profile_getPrimaryFont(self);
  if (primaryFont == NULL)
    return -1;
  return ttoy_Font_getSize(primaryFont);
}

ttoy_ErrorCode
ttoy_Profile_setFontSize(
    ttoy_Profile *self,
    float fontSize)
{
  ttoy_Font *primaryFont;
  ttoy_ErrorCode error;

  /* TODO: Change the size of the primary font */
  primaryFont = ttoy_Profile_getPrimaryFont(self);
  if (primaryFont == NULL) {
    return TTOY_ERROR_PROFILE_NO_PRIMARY_FONT;
  }
  error = ttoy_Font_setSize(primaryFont, fontSize);
  if (error != TTOY_NO_ERROR) {
    return error;
  }

  /* Adjust all of the fallback font sizes now that the primary font has
   * changed size */
  for (size_t i = 1;
      i < ttoy_FontRefArray_size(&self->internal->fonts);
      ++i)
  {
    ttoy_Font *font;
    font = ttoy_FontRef_get(ttoy_FontRefArray_get(&self->internal->fonts, i));
    error = ttoy_Profile_adjustFallbackFontSize(self, font);
    if (error != TTOY_NO_ERROR) {
      TTOY_LOG_ERROR(
          "Profile '%s' failed to adjust size for fallback font '%s'",
          self->name,
          ttoy_Font_getFontPath(font));
      /* NOTE: Not a fatal error; we might have some poorly rendered glyphs
       * though */
    }
  }
  for (size_t i = 0;
      i < ttoy_FontRefArray_size(&self->internal->boldFonts);
      ++i)
  {
    ttoy_Font *font;
    /* FIXME: Shouldn't we just set the bold font to the same size as the
     * normal font? */
    font = ttoy_FontRef_get(ttoy_FontRefArray_get(&self->internal->fonts, i));
    ttoy_Profile_adjustFallbackFontSize(self, font);
  }

  return TTOY_NO_ERROR;
}

ttoy_ErrorCode
ttoy_Profile_adjustFallbackFontSize(
    ttoy_Profile *self,
    ttoy_Font *font)
{
  /* TODO: Actually adjust the fallback font sizes (once we have fallback
   * fonts) */
  return TTOY_NO_ERROR;
}

void
ttoy_Profile_getFonts(
    ttoy_Profile *self,
    ttoy_FontRefArray *fonts,
    ttoy_FontRefArray *boldFonts)
{
  /* Copy all of our fonts to the given fonts array, incrementing reference
   * counts along the way */
#define COPY_FONT_REFS(ARRAY) \
  for (size_t i = 0; \
      i < ttoy_FontRefArray_size(&self->internal->ARRAY); \
      ++i) \
  { \
    ttoy_FontRef *fontRef; \
    fontRef = ttoy_FontRefArray_get(&self->internal->ARRAY, i); \
    ttoy_FontRefArray_append(ARRAY, fontRef); \
    ttoy_FontRef_increment(fontRef); \
  }
  COPY_FONT_REFS(fonts)
  COPY_FONT_REFS(boldFonts)
}

void ttoy_Profile_setBackgroundToy(
    ttoy_Profile *self,
    ttoy_BackgroundToy *backgroundToy)
{
  self->internal->backgroundToy = backgroundToy;
}

ttoy_BackgroundToy *
ttoy_Profile_getBackgroundToy(
    ttoy_Profile *self)
{
  return self->internal->backgroundToy;
}

void ttoy_Profile_setTextToy(
    ttoy_Profile *self,
    ttoy_TextToy *textToy)
{
  self->internal->textToy = textToy;
}

ttoy_TextToy *
ttoy_Profile_getTextToy(
    ttoy_Profile *self)
{
  return self->internal->textToy;
}
