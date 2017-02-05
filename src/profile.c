#include <assert.h>
#include <fontconfig/fontconfig.h>
#include <stdlib.h>
#include <string.h>

#include "fonts.h"
#include "profile.h"

struct st_Profile_Internal_ {
  st_BackgroundToy *backgroundToy;
  st_TextToy *textToy;
};

void st_Profile_init(
    st_Profile *self,
    const char *name)
{
  self->fontFace = NULL;
  self->fontPath = NULL;
  self->fontSize = 0.0f;
  /* Allocate memory for internal structures */
  self->internal = (st_Profile_Internal *)malloc(sizeof(st_Profile_Internal));
  self->internal->backgroundToy = NULL;
  self->internal->textToy = NULL;
  /* Copy the name string */
  self->name = (char *)malloc(strlen(name) + 1);
  strcpy(self->name, name);
}

void st_Profile_destroy(
    st_Profile *self)
{
  /* Free memory from internal structures */
  /* NOTE: Ownership of the toys is held by the st_Config object */
  free(self->internal);
  free(self->name);
  free(self->fontFace);
  free(self->fontPath);
}

void st_Profile_setFlags(
    st_Profile *self,
    uint32_t flags)
{
  /* TODO: We might need to do some processing when some of these flags change.
   * For now, there is nothing to do here. */
  self->flags = flags;
}

st_ErrorCode st_Profile_setFont(
    st_Profile *self,
    const char *fontFace,
    float fontSize)
{
  FcPattern *pattern;
  FcFontSet *sourceFontSets[2];
  FcFontSet *resultFontSet;
  FcConfig *fc;
  FcResult result;
  FcValue value;

  /* Use fontconfig to look for a font with this face and font size */
  fc = st_Fonts_getFontconfigInstance();

  sourceFontSets[0] = FcConfigGetFonts(fc, FcSetSystem);
  sourceFontSets[1] = FcConfigGetFonts(fc, FcSetApplication);

  pattern = FcPatternBuild(
      NULL,  /* pattern */
      FC_SPACING, FcTypeInteger, FC_MONO,  /* Look for monospace fonts... */
//      FC_PIXEL_SIZE, FcTypeDouble, fontSize,  /* ...with this pixel size... */
      FC_FULLNAME, FcTypeString, fontFace,  /* ...and with this font face. */
      (char *) NULL  /* terminator */
      );

  /* Generate a list of all fonts matching our criteria */
  resultFontSet = FcFontSetList(
      fc,  /* config */
      sourceFontSets,  /* sets */
      2,  /* nsets */
      pattern,  /* pattern */
      /* FIXME: Fontconfig documentation is not clear on the utility of
       * object_set */
      NULL  /* object_set */
      );
  FcPatternDestroy(pattern);

  /* XXX: Print out all of the matching fonts */
  for (int i = 0; i < resultFontSet->nfont; ++i) {
    FcPattern *font;
    font = resultFontSet->fonts[i];
    FcPatternPrint(font);
  }

  if (resultFontSet->nfont <= 0) {
    fprintf(stderr, "Error: Fontconfig could not find any suitable fonts.\n");
    FcFontSetDestroy(resultFontSet);
    return ST_ERROR_FONT_NOT_FOUND;
  }

  /* Get the file path of the matching font */
  result = FcPatternGet(resultFontSet->fonts[0], FC_FILE, 0, &value);
  if (result != FcResultMatch) {
    FcFontSetDestroy(resultFontSet);
    return ST_ERROR_FONT_NOT_FOUND;
  }
  assert(value.u.s != NULL);

  /* Replace the old font path with the new one */
  char *newFontPath = (char *)malloc(
      strlen((const char *)value.u.s) + 1);
  if (newFontPath == NULL) {
    FcFontSetDestroy(resultFontSet);
    return ST_ERROR_OUT_OF_MEMORY;
  }
  free(self->fontPath);
  self->fontPath = newFontPath;
  strcpy(
      self->fontPath,
      (const char *)value.u.s);
  fprintf(stderr, "Fontconfig found suitable font: '%s'\n",
      self->fontPath);
  FcFontSetDestroy(resultFontSet);

  /* Store the new font size and face name */
  self->fontSize = fontSize;
  if ((self->fontFace == NULL) || (strcmp(self->fontFace, fontFace) != 0)) {
    free(self->fontFace);
    self->fontFace = malloc(strlen(fontFace) + 1);
    strcpy(self->fontFace, fontFace);
  }

  return ST_NO_ERROR;
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
