/*
 * Copyright (c) 2016-2017 Jonathan Glines
 * Jonathan Glines <jonathan@glines.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <assert.h>
#include <errno.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fontconfig/fontconfig.h>
#include <unistd.h>

#include "./common/mkdir.h"
#include "backgroundToyDictionary.h"
#include "fonts.h"
#include "logging.h"
#include "textToyDictionary.h"
#include "toyFactory.h"

#include "config.h"

/* Private methods */
void st_Config_parseConfig(
    st_Config *self,
    const char *config,
    size_t len);
int st_Config_buildConfig(
    st_Config *self,
    json_t *root);
st_ErrorCode st_Config_buildProfile(
    st_Config *self,
    st_Profile *profile,
    json_t *profile_json);
st_ErrorCode st_Config_buildColorScheme(
    st_Config *self,
    st_Profile *profile,
    json_t *colorScheme_json);
st_ErrorCode st_Config_readConfigFile(
    st_Config *self);
st_ErrorCode st_Config_insertNewProfile(
    st_Config *self,
    st_Profile *newProfile);
st_ErrorCode st_Config_serialize(
    const st_Config *self,
    const char *path);

struct st_Config_Internal {
  /* FIXME: Since we are returning pointers to these profiles, this might be
   * better as a linked list. */
  /* FIXME: Replace profiles with a dictionary */
  st_Profile **profiles;
  char *defaultProfile, *fontFilePath;
  size_t sizeProfiles, numProfiles;
  st_BackgroundToyDictionary backgroundToys;
  st_TextToyDictionary textToys;
  st_ToyFactory toyFactory;
};

#define INIT_SIZE_PROFILES 4
#define INIT_SIZE_TOYS 4

void st_Config_init(
    st_Config *self)
{
  self->configFilePath = NULL;
  /* Allocate memory for internal data structures */
  self->internal = (struct st_Config_Internal *)malloc(
      sizeof(struct st_Config_Internal));
  self->internal->sizeProfiles = INIT_SIZE_PROFILES;
  self->internal->profiles = (st_Profile **)malloc(
      sizeof(st_Profile *) * INIT_SIZE_PROFILES);;
  self->internal->defaultProfile = NULL;
  self->internal->numProfiles = 0;
  self->internal->fontFilePath = NULL;
  /* Initialize the toy factory */
#define DEFAULT_PLUGIN_PATH "/usr/lib/shelltoy/plugins/"
  st_ToyFactory_init(
      &self->internal->toyFactory,
      DEFAULT_PLUGIN_PATH);
  /* Initialize our toy dictionaries */
  st_BackgroundToyDictionary_init(&self->internal->backgroundToys);
  st_TextToyDictionary_init(&self->internal->textToys);
}

void st_Config_destroy(
    st_Config *self)
{
  /* Destroy all of the toys in our toy dictionaries */
  for (size_t i = 0;
      i < st_BackgroundToyDictionary_size(&self->internal->backgroundToys);
      ++i)
  {
    st_BackgroundToy *backgroundToy;
    backgroundToy = st_BackgroundToyDictionary_getValueAtIndex(
        &self->internal->backgroundToys,
        i);
    st_BackgroundToy_destroy(backgroundToy);
    free(backgroundToy);
  }
  for (size_t i = 0;
      i < st_TextToyDictionary_size(&self->internal->textToys);
      ++i)
  {
    st_TextToy *textToy;
    textToy = st_TextToyDictionary_getValueAtIndex(
        &self->internal->textToys,
        i);
    st_TextToy_destroy(textToy);
    free(textToy);
  }
  /* Destroy all of our toy dictionaries */
  st_BackgroundToyDictionary_destroy(&self->internal->backgroundToys);
  st_TextToyDictionary_destroy(&self->internal->textToys);
  /* Destroy each of the held profiles */
  for (size_t i = 0; i < self->internal->numProfiles; ++i) {
    st_Profile_destroy(self->internal->profiles[i]);
  }
  /* Free memory allocated for internal data structures */
  /* FIXME: We never free memory allocated for toys */
  free(self->internal->fontFilePath);
  free(self->internal->profiles);
  free(self->internal->defaultProfile);
  free(self->internal);
  free(self->configFilePath);
}

void st_Config_setPluginPath(
    st_Config *self,
    const char *pluginPath)
{
  st_ToyFactory_setPluginPath(
      &self->internal->toyFactory,
      pluginPath);
}

st_ErrorCode st_Config_findConfigFile(
    st_Config *self)
{
  size_t size, home_len, suffix_len;
  char *path;
  const char *home;
  const char *suffix = "/.config/shelltoy/config.json";

  /* Check for existence of $HOME/.config/shelltoy/config.json */
  home = getenv("HOME");
  if (home == NULL) {
    ST_LOG_ERROR("%s", "HOME environment variable not set");
    return ST_ERROR_CONFIG_FILE_NOT_FOUND;
  }
  home_len = strlen(home);
  suffix_len = strlen(suffix);
  size = home_len + suffix_len + 1;
  path = (char *)malloc(size);
  strcpy(path, home);
  strcpy(path + home_len, suffix);
  if (access(path, R_OK) != 0) {
    ST_LOG_ERROR("Could not access config file '%s': %s",
        path,
        strerror(errno));
    free(path);
    return ST_ERROR_CONFIG_FILE_NOT_FOUND;
  }
  /* TODO: Check other possible paths for the config file */
  /* TODO:
  ST_LOG_DEBUG("Found config file '%s'",
      path);
  */
  free(self->configFilePath);
  self->configFilePath = path;
  /* Now that we have a new config file path, attempt to read from that file
   * and return the resulting error code. */
  return st_Config_readConfigFile(self);
}

st_ErrorCode st_Config_setConfigFilePath(
    st_Config *self,
    const char *path)
{
  char *newPath;
  newPath = (char *)malloc(strlen(path) + 1);
  strcpy(newPath, path);
  free(self->configFilePath);
  self->configFilePath = newPath;
  /* Now that we have a new config file path, attempt to read from that file
   * and return the resulting error code. */
  return st_Config_readConfigFile(self);
}

st_ErrorCode st_Config_readConfigFile(
    st_Config *self)
{
  FILE *fp;
  char *config;
  int result;
  long len;

  if (self->configFilePath == NULL) {
    ST_LOG_ERROR("Tried to read config file, but config file path not set",
        strerror(errno));
    return ST_ERROR_CONFIG_FILE_PATH_NOT_SET;
  }

  /* Attempt to open the file at the given path */
  fp = fopen(self->configFilePath, "r");
  if (fp == NULL) {
    ST_LOG_ERROR("Error opening config file: %s",
        strerror(errno));
    return ST_ERROR_CONFIG_FILE_READ;
  }

  /* Seek to the end of the config file to determine its length */
  result = fseek(fp, 0, SEEK_END);
  if (result) {
    fprintf(stderr, "Error opening config file: %s\n",
        strerror(errno));
    return ST_ERROR_CONFIG_FILE_READ;
  }
  len = ftell(fp);
  if (len < 0) {
    fprintf(stderr, "Error opening config file: %s\n",
        strerror(errno));
    /* FIXME: Fail gracefully here */
    assert(0);
  }
  rewind(fp);

  /* Read the entire config file into a string */
  config = (char *)malloc(len + 1);
  if (fread(config, 1, len, fp) < len) {
    if (ferror(fp)) {
      fprintf(stderr, "Error reading config file: %s\n",
          strerror(errno));
      /* FIXME: Fail gracefully here */
      assert(0);
    } else {
      /* This should not happen either */
      assert(0);
    }
  }
  config[len] = '\0';

  st_Config_parseConfig(self, config, len);

  fclose(fp);

  return ST_NO_ERROR;
}

void st_Config_parseConfig(
    st_Config *self,
    const char *config,
    size_t len)
{
  int error;
  json_t *root;
  json_error_t error_json;

  /* FIXME: Many of the calls to json_is_null() within st_Config_parseConfig
   * should actually be checks for null json_t pointers. Null is not the same
   * as non-existent in jansson. */

  root = json_loads(config, 0, &error_json);
  if (root == NULL) {
    fprintf(stderr,
        "Config error on line %d: %s\n"
        "Failed to parse JSON config.\n",
        error_json.line, error_json.text);
    /* Fail gracefully */
    exit(EXIT_FAILURE);
  }

  error = st_Config_buildConfig(self, root);
  json_decref(root);
  if (error) {
    /* Fail gracefully */
    ST_LOG_ERROR_CODE(error);
    exit(EXIT_FAILURE);
  }
}

int comp_profiles(
    const st_Profile **a,
    const st_Profile **b)
{
  return strcmp((*a)->name, (*b)->name);
}

int comp_backgroundToys(
    const st_BackgroundToy **a,
    const st_BackgroundToy **b)
{
  return strcmp((*a)->name, (*b)->name);
}

int st_Config_buildConfig(
    st_Config *self,
    json_t *root)
{
  json_t *defaultProfile_json, *profiles, *plugins, *toys;
  st_ErrorCode error;

  if (!json_is_object(root)) {
    fprintf(stderr, "Config error: root is not an object\n");
    return 1;
  }

  /* Traverse the root JSON object to set values in our config */

  /* Retrieve the array of plugins */
  plugins = json_object_get(root, "plugins");
  if (plugins == NULL || json_is_null(plugins)) {
    /* No plugins specified; this is okay */
  } else if (!json_is_array(plugins)) {
    ST_LOG_ERROR("%s", "Config error: plugins is not an array");
    return ST_ERROR_CONFIG_FILE_FORMAT;
  } else {
    /* Iterate to register each plugin with the toy factory */
    for (size_t i = 0; i < json_array_size(plugins); ++i) {
      json_t *plugin_json, *name_json, *file_json;
      plugin_json = json_array_get(plugins, i);
      if (!json_is_object(plugin_json)) {
        ST_LOG_ERROR("%s", "Config error: each plugin must be a JSON object");
        return ST_ERROR_CONFIG_FILE_FORMAT;
      }
      name_json = json_object_get(plugin_json, "name");
      if (json_is_null(name_json)) {
        ST_LOG_ERROR("%s", "Config error: each plugin must have a name");
        return ST_ERROR_CONFIG_FILE_FORMAT;
      } else if (!json_is_string(name_json)) {
        ST_LOG_ERROR("%s", "Config error: plugin name must be a string");
        return ST_ERROR_CONFIG_FILE_FORMAT;
      }
      file_json = json_object_get(plugin_json, "file");
      if (json_is_null(file_json)) {
        ST_LOG_ERROR("%s", "Config error: each plugin must have a file");
        return ST_ERROR_CONFIG_FILE_FORMAT;
      } else if (!json_is_string(file_json)) {
        ST_LOG_ERROR("%s", "Config error: plugin file must be given as a string");
        return ST_ERROR_CONFIG_FILE_FORMAT;
      }
      error = st_ToyFactory_registerPlugin(
          &self->internal->toyFactory,
          json_string_value(name_json),  /* name */
          json_string_value(file_json)  /* dlPath */
          );
      if (error == ST_ERROR_PLUGIN_DL_FAILED_TO_LOAD) {
        ST_LOG_ERROR(
            "Config error: Failed to load dynamic library '%s' for plugin '%s'",
            json_string_value(file_json),
            json_string_value(name_json));
      } else if (error != ST_NO_ERROR) {
        ST_LOG_ERROR(
            "Config error: Failed to load plugin '%s'",
            json_string_value(name_json));
        ST_LOG_ERROR_CODE(error);
      }
    }
  }

  /* Retrieve the array of toys */
  toys = json_object_get(root, "toys");
  if (toys == NULL || json_is_null(toys)) {
    /* No toys specified; this is okay */
  } else if (!json_is_array(toys)) {
    ST_LOG_ERROR("%s", "Config error: toys is not an array");
    return ST_ERROR_CONFIG_FILE_FORMAT;
  } else {
    /* Iterate to build each toy with the toy factory */
    for (size_t i = 0; i < json_array_size(toys); ++i) {
      json_t *toy_json, *pluginName_json, *toyName_json, *toyType_json,
             *toyConfig_json;

      toy_json = json_array_get(toys, i);
      if (!json_is_object(toy_json)) {
        ST_LOG_ERROR("%s", "Config error: each toy must be a JSON object");
        return ST_ERROR_CONFIG_FILE_FORMAT;
      }
      pluginName_json = json_object_get(toy_json, "plugin");
      if (!json_is_string(pluginName_json)) {
        ST_LOG_ERROR("%s", "Config error: plugin name must be a string");
        return ST_ERROR_CONFIG_FILE_FORMAT;
      }
      toyName_json = json_object_get(toy_json, "name");
      if (!json_is_string(toyName_json)) {
        ST_LOG_ERROR("%s", "Config error: toy name must be a string");
        return ST_ERROR_CONFIG_FILE_FORMAT;
      }
      toyType_json = json_object_get(toy_json, "type");
      if (!json_is_string(toyType_json)) {
        ST_LOG_ERROR("%s", "Config error: toy type must be a string");
        return ST_ERROR_CONFIG_FILE_FORMAT;
      }
      toyConfig_json = json_object_get(toy_json, "config");
      if (strcmp(json_string_value(toyType_json), "background") == 0) {
        st_BackgroundToy *toy;
        /* This is a background toy */
        error = st_ToyFactory_buildBackgroundToy(
            &self->internal->toyFactory,
            json_string_value(pluginName_json),  /* pluginName */
            json_string_value(toyName_json),  /* toyName */
            toyConfig_json,  /* config */
            &toy  /* toy */
            );
        if (error == ST_ERROR_PLUGIN_NOT_FOUND) {
        } else {
        /* Insert this toy into our background toy dictionary */
        st_BackgroundToyDictionary_insert(
            &self->internal->backgroundToys,
            json_string_value(toyName_json),  /* key */
            toy  /* value */
            );
        }
      } else if (strcmp(json_string_value(toyType_json), "text") == 0) {
        st_TextToy *toy;
        /* This is a text toy */
        error = st_ToyFactory_buildTextToy(
            &self->internal->toyFactory,
            json_string_value(pluginName_json),  /* pluginName */
            json_string_value(toyName_json),  /* toyName */
            toyConfig_json,  /* config */
            &toy  /* toy */
            );
        if (error == ST_ERROR_PLUGIN_DL_FAILED_TO_LOAD) {
          ST_LOG_ERROR(
              "Config error: Could not find plugin '%s' for toy '%s'",
              json_string_value(pluginName_json),
              json_string_value(toyName_json));
        } else if (error != ST_NO_ERROR) {
          ST_LOG_ERROR(
              "Config error: Failed to load toy '%s'",
              json_string_value(toyName_json));
          ST_LOG_ERROR_CODE(error);
        } else {
          /* Insert this toy into our text toy dictionary */
          st_TextToyDictionary_insert(
              &self->internal->textToys,
              json_string_value(toyName_json),  /* key */
              toy  /* value */
              );
        }
      } else {
        ST_LOG_ERROR("Config error: Unknown toy type '%s'",
            json_string_value(toyType_json));
        return ST_ERROR_CONFIG_FILE_FORMAT;
      }
    }
    /* TODO: Should we prohibit background toys with the same names as text
     * toys? */
  }

  /* Retrieve the array of profiles */
  profiles = json_object_get(root, "profiles");
  if (!json_is_array(profiles)) {
    ST_LOG_ERROR("%s", "Config error: profiles is not an array\n");
    return ST_ERROR_CONFIG_FILE_FORMAT;
  }
  /* Allocate memory for storing our profiles */
  self->internal->sizeProfiles = json_array_size(profiles);
  self->internal->profiles = (st_Profile **)malloc(
      sizeof(st_Profile *) * self->internal->sizeProfiles);
  /* Iterate to add each profile */
  assert(self->internal->numProfiles == 0);
  for (size_t i = 0; i < json_array_size(profiles); ++i) {
    json_t *profile_json;
    st_Profile *profile;

    /* Build the profile from its JSON representation */
    profile_json = json_array_get(profiles, i);
    profile = (st_Profile *)malloc(sizeof(st_Profile));
    self->internal->profiles[self->internal->numProfiles++] = profile;
    error = st_Config_buildProfile(self,
        profile,
        profile_json);
    if (error != ST_NO_ERROR)
      return error;
  }
  /* Sort the profiles by name */
  qsort(
      self->internal->profiles,  /* ptr */
      self->internal->numProfiles,  /* count */
      sizeof(st_Profile *),  /* size */
      (int (*)(const void *, const void *))comp_profiles  /* comp */
      );

  /* Retrieve the default profile */
  defaultProfile_json = json_object_get(root, "defaultProfile");
  if (json_is_null(defaultProfile_json)) {
    /* No default profile set */
    self->internal->defaultProfile = NULL;
  } else if (!json_is_string(defaultProfile_json)) {
    fprintf(stderr, "Config error: defaultProfile is not a string\n");
    /* FIXME: This might not be an error if the user actually specified a
     * profile to use */
    return ST_ERROR_CONFIG_FILE_FORMAT;
  } else {
    /* Copy the default profile string */
    assert(self->internal->defaultProfile == NULL);
    const char *defaultProfile = json_string_value(defaultProfile_json);
    self->internal->defaultProfile = (char *)malloc(strlen(defaultProfile) + 1);
    strcpy(self->internal->defaultProfile, defaultProfile);
  }

  return 0;
}

st_ErrorCode
st_Config_buildProfile(
    st_Config *self,
    st_Profile *profile,
    json_t *profile_json)
{
  json_t *name, *fontFace, *fallbackFontFaces, *fontSize, *antialiasFont,
      *brightIsBold, *colors, *background;
  uint32_t flags;
  st_ErrorCode error;

  if (!json_is_object(profile_json)) {
    fprintf(stderr, "Config error: profile is not an object\n");
    return ST_ERROR_CONFIG_FILE_FORMAT;
  }

  /* Get the name of this profile */
  name = json_object_get(profile_json, "name");
  if (name == NULL) {
    fprintf(stderr, "Config error: profile is missing a name\n");
    return ST_ERROR_CONFIG_FILE_FORMAT;
  }
  if (!json_is_string(name)) {
    fprintf(stderr, "Config error: profile name is not a string\n");
    return ST_ERROR_CONFIG_FILE_FORMAT;
  }
  st_Profile_init(profile,
      json_string_value(name)  /* name */
      );

  /* Check for profile strings */
#define CHECK_PROFILE_STRING(key) \
  key = json_object_get(profile_json, #key); \
  if (key == NULL) { \
    fprintf(stderr, "Config error: profile '%s' missing value for '%s'", \
        profile->name, \
        #key); \
    return ST_ERROR_CONFIG_FILE_FORMAT; \
  } \
  if (!json_is_string(key)) { \
    fprintf(stderr, \
        "Config error: value of '%s' in profile '%s' is not a string\n", \
        #key, \
        profile->name); \
    return ST_ERROR_CONFIG_FILE_FORMAT; \
  }
  CHECK_PROFILE_STRING(fontFace)

  /* Check for numerical profile values */
  fontSize = json_object_get(profile_json, "fontSize");
  if (!json_is_real(fontSize)) {
    fprintf(stderr, "Config error: profile fontSize is not a number\n");
    return 1;
  }
  profile->fontSize = (float)json_real_value(fontSize);

  /* Get the profile flags */
  flags = 0;
#define GET_PROFILE_FLAG(key,flag) \
  key = json_object_get(profile_json, #key); \
  if (!json_is_boolean(key)) { \
    fprintf(stderr, "Config error: profile " #key " is not a boolean\n"); \
    return 1; \
  } \
  flags |= json_boolean_value(key) ? ST_PROFILE_ ## flag : 0;
  GET_PROFILE_FLAG(antialiasFont, ANTIALIAS_FONT);
  GET_PROFILE_FLAG(brightIsBold, BRIGHT_IS_BOLD);
  st_Profile_setFlags(profile, flags);

  /* Get the profile colors */
  colors = json_object_get(profile_json, "colors");
  error = st_Config_buildColorScheme(self,
      profile,
      colors);
  if (error != ST_NO_ERROR) {
    ST_LOG_ERROR("%s", "Error reading profile colors");
    return error;
  }

  /* Set the primary font used by the profile. Note that failure to find the
   * font is not a fatal error, since we might not even use this profile. */
  error = st_Profile_setPrimaryFont(profile,
      json_string_value(fontFace),  /* fontFace */
      (float)json_real_value(fontSize)  /* fontSize */
      );
  if (error != ST_NO_ERROR) {
    ST_LOG_ERROR_CODE(error);
    ST_LOG_ERROR("Could not load font face '%s' in size '%f' for profile '%s'",
        json_string_value(fontFace),
        (float)json_real_value(fontSize),
        json_string_value(name));
  }

  /* Iterate through all of the fallback fonts */
  fallbackFontFaces = json_object_get(profile_json, "fallbackFontFaces");
  if (fallbackFontFaces == NULL || json_is_null(fallbackFontFaces)) {
    /* No fallback font faces specified; this is okay */
  } else if (!json_is_array(fallbackFontFaces)) {
      ST_LOG_ERROR("Fallback font faces must be in an array in profile '%s'",
          json_string_value(name));
      return ST_ERROR_CONFIG_FILE_FORMAT;
  } else {
    for (size_t i = 0; i < json_array_size(fallbackFontFaces); ++i) {
      json_t *fallbackFontFace;
      fallbackFontFace = json_array_get(fallbackFontFaces, i);
      if (!json_is_string(fallbackFontFace)) {
        ST_LOG_ERROR("Fallback font faces must be strings in profile '%s'",
            json_string_value(name));
        return ST_ERROR_CONFIG_FILE_FORMAT;
      }
      /* Add this fallback font face to the profile */
      error = st_Profile_addFallbackFont(profile,
          json_string_value(fallbackFontFace));
      if (error != ST_NO_ERROR) {
        ST_LOG_ERROR_CODE(error);
        ST_LOG_ERROR("Could not load fallback font face '%s' for profile '%s'",
            json_string_value(fallbackFontFace),
            json_string_value(name));
        /* NOTE: Missing fallback fonts is a non-fatal error */
      }
    }
  }

  /* Get the background toy used by this profile */
  background = json_object_get(profile_json, "background");
  if (background == NULL || json_is_null(background)) {
    /* The background was not specified; this is okay */
  } else if (json_is_object(background)) {
    json_t *toyName;
    st_BackgroundToy *backgroundToy;
    toyName = json_object_get(background, "toy");
    if (!json_is_string(toyName)) {
      ST_LOG_ERROR("Toy name must be a string in background of profile '%s'",
          json_string_value(name));
      return ST_ERROR_CONFIG_FILE_FORMAT;
    }
    backgroundToy = st_BackgroundToyDictionary_getValue(
        &self->internal->backgroundToys,
        json_string_value(toyName)  /* key */
        );
    if (backgroundToy == NULL) {
      ST_LOG_ERROR(
          "Toy '%s' referenced in background of profile '%s' does not exist",
          json_string_value(toyName),
          json_string_value(name));
      return ST_ERROR_CONFIG_FILE_FORMAT;
    }
    st_Profile_setBackgroundToy(profile, backgroundToy);
  } else {
    ST_LOG_ERROR("Background must be a JSON object in profile '%s'",
        json_string_value(name));
    return ST_ERROR_CONFIG_FILE_FORMAT;
  }

  return ST_NO_ERROR;
}

st_ErrorCode st_Config_buildColorScheme(
    st_Config *self,
    st_Profile *profile,
    json_t *colorScheme_json)
{
  json_t *color_json;

  if (!json_is_object(colorScheme_json)) {
    ST_LOG_ERROR("%s", "Config error: colors is not an object");
    return ST_ERROR_CONFIG_FILE_FORMAT;
  }

  /* TODO: Get rid of this huge macro and make an st_Config_buildColor()
   * subroutine */
#define GET_COLOR(NAME,CODE) \
  color_json = json_object_get(colorScheme_json, #NAME); \
  /* Parse the color as it is represented in JSON */ \
  if (json_is_array(color_json)) { \
    if (json_array_size(color_json) != 3) { \
      ST_LOG_ERROR("Color '%s' in profile '%s' should have 3 components", \
          #NAME, \
          profile->name); \
      return ST_ERROR_CONFIG_FILE_FORMAT; \
    } \
    for (size_t i = 0; i < json_array_size(color_json); ++i) { \
      json_t *channel_json; \
      json_int_t channel; \
      channel_json = json_array_get(color_json, i); \
      if (!json_is_integer(channel_json)) { \
        ST_LOG_ERROR("Color '%s' in profile '%s' contains colors that are not integers", \
            #NAME, \
            profile->name); \
        return ST_ERROR_CONFIG_FILE_FORMAT; \
      } \
      channel = json_integer_value(channel_json); \
      if ((channel < 0) || (255 < channel)) { \
        ST_LOG_ERROR("Color '%s' in profile '%s': color values must be between 0 and 255", \
            #NAME, \
            profile->name); \
        return ST_ERROR_CONFIG_FILE_FORMAT; \
      } \
      profile->colorScheme.colors[ST_COLOR_ ## CODE].rgb[i] = (uint8_t)channel; \
    } \
  } else if (json_is_string(color_json)) { \
    /* TODO: Parse CSS-style color strings */ \
    assert(0); \
  } else if (json_is_null(color_json)) { \
    /* TODO: Change this error to a warning and load the default color for any
     * missing colors */ \
    ST_LOG_ERROR("Missing color '%s' in profile '%s'", \
        #NAME, \
        profile->name); \
    return ST_ERROR_CONFIG_FILE_FORMAT; \
  } \
  else { \
    ST_LOG_ERROR("Could not parse color '%s' in profile '%s'", \
        #NAME, \
        profile->name); \
    return ST_ERROR_CONFIG_FILE_FORMAT; \
  }
  GET_COLOR(color0,0)
  GET_COLOR(color1,1)
  GET_COLOR(color2,2)
  GET_COLOR(color3,3)
  GET_COLOR(color4,4)
  GET_COLOR(color5,5)
  GET_COLOR(color6,6)
  GET_COLOR(color7,7)
  GET_COLOR(color8,8)
  GET_COLOR(color9,9)
  GET_COLOR(color10,10)
  GET_COLOR(color11,11)
  GET_COLOR(color12,12)
  GET_COLOR(color13,13)
  GET_COLOR(color14,14)
  GET_COLOR(color15,15)
  GET_COLOR(background,BACKGROUND)
  GET_COLOR(foreground,FOREGROUND)

  return ST_NO_ERROR;
}

/*
st_ErrorCode st_Config_BuildColor(
    st_Config *self,
    uint8_t *color,
    json_t *color_json)
{
}
*/

st_ErrorCode
st_Config_getProfile(
    st_Config *self,
    const char *name,
    st_Profile **profile)
{
  int a, b, i, result;

  assert(name != NULL);

  if (self->internal->numProfiles == 0)
    return ST_ERROR_PROFILE_NOT_FOUND;

  /* Look for a profile with the given name using a binary search */
  a = 0; b = self->internal->numProfiles;
  i = -1;
  while (a < b) {
    i = (b - a) / 2 + a;
    *profile = self->internal->profiles[i];
    result = strcmp(name, (*profile)->name);
    if (result < 0) {
      b = i;
    } else if (result > 0) {
      a = i + 1;
    } else {
      /* Found the profile */
      break;
    }
  }
  *profile = self->internal->profiles[i];
  result = strcmp(name, (*profile)->name);
  if (result != 0) {
    *profile = NULL;
    return ST_ERROR_PROFILE_NOT_FOUND;
  }
  return ST_NO_ERROR;
}

st_ErrorCode
st_Config_getDefaultProfile(
    st_Config *self,
    st_Profile **profile)
{
  st_ErrorCode error;

  if (self->internal->defaultProfile == NULL) {
#define DEFAULT_PROFILE_NAME "default"
#define DEFAULT_FONT_FACE "DejaVu Sans Mono"
#define DEFAULT_FONT_SIZE 12.0
#define DEFAULT_FLAGS ( \
    ST_PROFILE_ANTIALIAS_FONT \
    | ST_PROFILE_BRIGHT_IS_BOLD) 
    /* The default profile has not yet been set, so we use "default" */
    self->internal->defaultProfile = (char *)malloc(
        strlen(DEFAULT_PROFILE_NAME) + 1);
    strcpy(self->internal->defaultProfile, DEFAULT_PROFILE_NAME);
    /* Look for a pre-existing profile with the name "default" */
    error = st_Config_getProfile(self,
        DEFAULT_PROFILE_NAME,  /* name */
        profile  /* profile */
        );
    if (error == ST_ERROR_PROFILE_NOT_FOUND) {
      st_Profile *newProfile;
      /* The "default" profile does not yet exist, so we create it */
      newProfile = (st_Profile *)malloc(sizeof(st_Profile));
      st_Profile_init(newProfile, DEFAULT_PROFILE_NAME);
      st_Profile_setFlags(newProfile, DEFAULT_FLAGS);
#define SET_COLOR(NAME,R,G,B) \
      newProfile->colorScheme.colors[ST_COLOR_ ## NAME].rgb[0] = R; \
      newProfile->colorScheme.colors[ST_COLOR_ ## NAME].rgb[1] = G; \
      newProfile->colorScheme.colors[ST_COLOR_ ## NAME].rgb[2] = B;
      SET_COLOR(0,0,0,0)  /* black */
      SET_COLOR(1,255,0,0)  /* light red */
      SET_COLOR(2,0,255,0)  /* light green */
      SET_COLOR(3,255,255,0)  /* yellow */
      SET_COLOR(4,0,0,255)  /* light blue */
      SET_COLOR(5,255,0,255)  /* light magenta */
      SET_COLOR(6,0,255,255)  /* light cyan */
      SET_COLOR(7,255,255,255)  /* high white */
      SET_COLOR(8,128,128,128)  /* gray */
      SET_COLOR(9,128,0,0)  /* red */
      SET_COLOR(10,0,128,0)  /* green */
      SET_COLOR(11,128,128,0)  /* brown */
      SET_COLOR(12,0,0,128)  /* blue */
      SET_COLOR(13,128,0,128)  /* magenta */
      SET_COLOR(14,0,128,128)  /* cyan */
      SET_COLOR(15,192,192,192)  /* white */
      SET_COLOR(FOREGROUND,255,255,255)  /* white */
      SET_COLOR(BACKGROUND,0,0,0)  /* black */
      error = st_Profile_setPrimaryFont(newProfile,
          DEFAULT_FONT_FACE,
          DEFAULT_FONT_SIZE);
      /* Insert the new profile in the array of profiles */
      error = st_Config_insertNewProfile(self,
          newProfile);
      ST_ASSERT_ERROR_CODE(error);
    } else if (error != ST_NO_ERROR) {
      ST_LOG_ERROR_CODE(error);
      ST_ASSERT_ERROR_CODE(error);
      return error;
    }
  }

  /* Look for the default profile */
  error = st_Config_getProfile(self,
      self->internal->defaultProfile,  /* name */
      profile  /* profile */
      );
  if (error == ST_ERROR_PROFILE_NOT_FOUND) {
    ST_LOG_ERROR("Default profile '%s' not found",
        self->internal->defaultProfile);
    return error;
  }

  return ST_NO_ERROR;
}

st_ErrorCode st_Config_insertNewProfile(
    st_Config *self,
    st_Profile *newProfile)
{
  /* Ensure that we have enough memory to store the address of this new
   * profile */
  if (self->internal->sizeProfiles < self->internal->numProfiles + 1) {
    st_Profile **newProfiles = (st_Profile **)malloc(
        sizeof(st_Profile *) * self->internal->sizeProfiles * 2);
    if (newProfiles == NULL) {
      return ST_ERROR_OUT_OF_MEMORY;
    }
    memcpy(
        newProfiles,
        self->internal->profiles,
        sizeof(st_Profile *) * self->internal->numProfiles);
    free(self->internal->profiles);
    self->internal->profiles = newProfiles;
    self->internal->sizeProfiles *= 2;
  }

  if (self->internal->numProfiles == 0) {
    self->internal->profiles[0] = newProfile;
    self->internal->numProfiles += 1;
    return ST_NO_ERROR;
  }

  /* Binary search for the sorted index of the new profile */
  int a = 0, b = self->internal->numProfiles, i;
  while (a < b) {
    i = (b - a) / 2 + a;
    int result = strcmp(
        self->internal->profiles[i]->name,
        newProfile->name);
    if (result < 0) {
      b = i;
    } else if (result > 0) {
      a = i + 1;
    } else {
      /* result == 0 */
      /* We should not be adding duplicate profiles */
      assert(0);
    }
  }
  /* Scan for the sorted index of the new profile */
  for (i = a; i < b; ++i) {
    int result = strcmp(
        self->internal->profiles[i]->name,
        newProfile->name);
    if (result < 0)
      break;
  }
#ifndef NDEBUG
  if (self->internal->numProfiles > 0) {
    if (i < self->internal->numProfiles) {
      int result = strcmp(
          self->internal->profiles[i]->name,
          newProfile->name);
      assert(result < 0);
    } else if (i == self->internal->numProfiles) {
    }
  }
#endif
  /* Move existing profiles to make room for the insert */
  memmove(
      &self->internal->profiles[i] + 1,
      &self->internal->profiles[i],
      sizeof(st_Profile *) * (self->internal->numProfiles - i));
  /* Insert the new profile */
  self->internal->profiles[i] = newProfile;
  self->internal->numProfiles += 1;

  return ST_NO_ERROR;
}

st_ErrorCode
st_Config_serialize(
    const st_Config *self,
    const char *path)
{
  /* FIXME: This function leaks jansson objects very badly, especially upon
   * error */
  st_PluginDictionary *plugins;
  json_t *root, *plugins_json;
  FILE *fp;
  int result;

  /* Create the root JSON object of the config */
  root = json_object();
  if (root == NULL) {
    return ST_ERROR_OUT_OF_MEMORY;
  }

  /* Build the array of plugins */
  plugins_json = json_array();
  if (plugins_json == NULL) {
    return ST_ERROR_OUT_OF_MEMORY;
  }
  /* Iterate over the plugins registered with our toy factory */
  plugins = st_ToyFactory_getPlugins(&self->internal->toyFactory);
  for (size_t i = 0;
      i < st_PluginDictionary_size(plugins);
      ++i)
  {
    st_Plugin *plugin;
    json_t *plugin_json, *name_json;

    plugin = st_PluginDictionary_getValueAtIndex(plugins, i);

    plugin_json = json_object();
    if (plugin_json == NULL) {
      return ST_ERROR_OUT_OF_MEMORY;
    }
    name_json = json_string(plugin->name);
    if (name_json == NULL) {
      return ST_ERROR_OUT_OF_MEMORY;
    }
    result = json_object_set(plugin_json, "name", name_json);
    if (result < 0) {
      ST_LOG_ERROR("%s", "Failed to set JSON plugin name");
      return ST_ERROR_CONFIG_FAILED_TO_SERIALIZE;
    }
  }
  result = json_object_set(root, "plugins", plugins_json);
  if (result < 0) {
    ST_LOG_ERROR("%s", "Failed to add plugins to JSON root object");
    return ST_ERROR_CONFIG_FAILED_TO_SERIALIZE;
  }

  /* TODO: Build the array of toys */

  /* TODO: Build the array of profiles */

  /* Open the config file for writing */
  fp = fopen(path, "w");
  if (fp == NULL) {
    ST_LOG_ERROR("Failed to open config file for writing: %s",
        strerror(errno));
    return ST_ERROR_CONFIG_FAILED_TO_SERIALIZE;
  }

  /* Write the JSON representation of our config to file */
  result = json_dumpf(
      root,  /* json */
      fp,  /* output */
      JSON_INDENT(4)  /* flags */
      );
  if (result < 0) {
    /* FIXME: This can result in corrupted JSON files... perhaps using
     * json_dumps() instead would be safer? */
    ST_LOG_ERROR("%s", "Failed to write JSON config file");
    return ST_ERROR_CONFIG_FAILED_TO_SERIALIZE;
  }
  return ST_NO_ERROR;
}

st_ErrorCode
st_Config_createDefaultConfigFile(
    const st_Config *self)
{
  struct stat sbuf;
  int result;
  const char *home;
  const char *suffix = "/.config/shelltoy/config.json";
  char *path;
  st_ErrorCode error;

  /* Check for existence of $HOME directory */
  home = getenv("HOME");
  if (home == NULL) {
    ST_LOG_ERROR("%s", "HOME environment variable not set");
    return ST_ERROR_FAILED_TO_CREATE_CONFIG_FILE;
  }
  result = stat(home, &sbuf);
  if (result != 0) {
    ST_LOG_ERROR("Could not stat HOME directory '%s': %s\n",
        home);
    return ST_ERROR_FAILED_TO_CREATE_CONFIG_FILE;
  }
  if (!S_ISDIR(sbuf.st_mode)) {
    ST_LOG_ERROR("HOME path '%s' is not a directory",
        home);
    return ST_ERROR_FAILED_TO_CREATE_CONFIG_FILE;
  }
  /* Recursively create $HOME/.config/shelltoy directory */
  error = st_mkdir(
      home,  /* working directory */
      ".config/shelltoy"  /* directory path */
      );
  if (error != ST_NO_ERROR) {
    return error;
  }
  /* Concatenate config file path */
  path = (char *)malloc(strlen(home) + strlen(suffix) + 1);
  strcpy(path, home);
  strcpy(path + strlen(home), suffix);
  /* Serialize a JSON representation of the current (presumably default)
   * config */
  st_Config_serialize(self, path);

  return ST_NO_ERROR;
}
