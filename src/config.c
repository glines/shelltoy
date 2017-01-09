/*
 * Copyright (c) 2016 Jonathan Glines
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

#include "config.h"
#include "fonts.h"
#include "logging.h"

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
st_ErrorCode st_Config_readConfigFile(
    st_Config *self);
st_ErrorCode st_Config_insertNewProfile(
    st_Config *self,
    st_Profile *newProfile);

struct st_Config_Internal {
  /* FIXME: Since we are returning pointers to these profiles, this might be
   * better as a linked list. */
  st_Profile **profiles;
  char *defaultProfile, *fontFilePath;
  size_t sizeProfiles, numProfiles;
};

#define INIT_SIZE_PROFILES 4

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
}

void st_Config_destroy(
    st_Config *self)
{
  /* Destroy each of the held profiles */
  for (size_t i = 0; i < self->internal->numProfiles; ++i) {
    st_Profile_destroy(self->internal->profiles[i]);
  }
  /* Free memory allocated for internal data structures */
  free(self->internal->fontFilePath);
  free(self->internal->profiles);
  free(self->internal->defaultProfile);
  free(self->internal);
  free(self->configFilePath);
}

st_ErrorCode st_Config_findConfigFile(
    st_Config *self)
{
  size_t size, home_len, suffix_len;
  char *path;
  const char *home;
  const char *suffix = "/.config/shelltoy/config.json";

  /* TODO: Check for existance of $HOME/.config/shelltoy/config.json */
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

  root = json_loads(config, 0, &error_json);
  if (root == NULL) {
    fprintf(stderr,
        "Config error on line %d: %s\n"
        "Failed to parse JSON config.\n",
        error_json.line, error_json.text);
    assert(0);
    /* FIXME: Fail gracefully */
  }

  error = st_Config_buildConfig(self, root);
  json_decref(root);
  if (error) {
    assert(0);
    /* FIXME: Fail gracefully */
  }
}

int st_Config_buildConfig(
    st_Config *self,
    json_t *root)
{
  json_t *defaultProfile_json, *profiles;
  st_ErrorCode error;

  if (!json_is_object(root)) {
    fprintf(stderr, "Config error: root is not an object\n");
    return 1;
  }

  /* Traverse the root JSON object to set values in our config */

  /* Retrieve the array of profiles */
  profiles = json_object_get(root, "profiles");
  if (!json_is_array(profiles)) {
    /* TODO: Add an error message for when profiles is missing */
    fprintf(stderr, "Config error: profiles is not an array\n");
    return 1;
  }

  /* Allocate memory for storing our profiles */
  self->internal->sizeProfiles = json_array_size(profiles);
  assert(self->internal->profiles == NULL);
  self->internal->profiles = (st_Profile **)malloc(
      sizeof(st_Profile *) * self->internal->sizeProfiles);
  /* Iterate to add each profile */
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
  /* TODO: Sort the profiles by name */

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
  json_t *name, *fontFace, *fontSize, *antialiasFont, *brightIsBold;
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
      json_string_value(name));

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

  /* TODO: Get the profile colors */

  /* Set the font used by the profile. Note that failure to find the font is
   * not a fatal error, since we might not even use this profile. */
  error = st_Profile_setFont(profile,
      json_string_value(fontFace),
      (float)json_real_value(fontSize));
  if (error != ST_NO_ERROR) {
    ST_LOG_ERROR("Could not find font face '%s' in size '%f' for profile '%s'",
        json_string_value(fontFace),
        (float)json_real_value(fontSize),
        json_string_value(name));
    ST_LOG_ERROR_CODE(error);
  }

  return ST_NO_ERROR;
}

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
#define DEFAULT_FONT_SIZE 14.0
#define DEFAULT_FLAGS ( \
    ST_PROFILE_ANTIALIAS_FONT \
    | ST_PROFILE_BRIGHT_IS_BOLD) 
    /* The default profile has not yet been set, so we use "default" */
    self->internal->defaultProfile = (char *)malloc(
        strlen(DEFAULT_PROFILE_NAME) + 1);;
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
      error = st_Profile_setFont(newProfile,
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
  memcpy(&self->internal->profiles[i], newProfile, sizeof(st_Profile *));

  return ST_NO_ERROR;
}
