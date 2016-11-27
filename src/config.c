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

#include "config.h"

void st_Config_readConfigFile(
    st_Config *self,
    const char *configFilePath);
void st_Config_parseConfig(
    st_Config *self,
    const char *config,
    size_t len);
int st_Config_buildConfig(
    st_Config *self,
    json_t *root);
int st_Config_buildProfile(
    st_Config *self,
    st_Profile *profile,
    json_t *profile_json);
const st_Profile *st_Config_getDefaultProfile(
    const st_Config *self);

struct st_Config_Internal {
  /* FIXME: Since we are returning pointers to these profiles, this might be
   * better as a linked list. */
  st_Profile *profiles;
  size_t sizeProfiles, numProfiles;
  size_t defaultProfileIndex;
};

void st_Config_init(
    st_Config *self,
    const char *configFilePath)
{
  /* TODO: Allocate memory for internal data structures */
  self->internal = (struct st_Config_Internal *)malloc(
      sizeof(struct st_Config_Internal));
  self->internal->sizeProfiles = 0;
  self->internal->profiles = NULL;
  self->internal->numProfiles = 0;
  self->internal->defaultProfileIndex = -1;

  st_Config_readConfigFile(self, configFilePath);
}

void st_Config_readConfigFile(
    st_Config *self,
    const char *configFilePath)
{
  FILE *fp;
  char *config;
  int result;
  long len;

  /* Attempt to open the file at the given path */
  fp = fopen(configFilePath, "r");
  if (fp == NULL) {
    fprintf(stderr, "Error opening config file: %s\n",
        strerror(errno));
    /* FIXME: Fail gracefully here */
    assert(0);
  }

  /* Seek to the end of the config file to determine its length */
  result = fseek(fp, 0, SEEK_END);
  if (result) {
    fprintf(stderr, "Error opening config file: %s\n",
        strerror(errno));
    /* FIXME: Fail gracefully here */
    assert(0);
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
  int error;

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
  self->internal->profiles = (st_Profile *)malloc(
      sizeof(st_Profile) * self->internal->sizeProfiles);
  /* Iterate to add each profile */
  for (size_t i = 0; i < json_array_size(profiles); ++i) {
    json_t *profile_json;
    st_Profile *profile;

    /* Build the profile from its JSON representation */
    profile_json = json_array_get(profiles, i);
    profile = &self->internal->profiles[self->internal->numProfiles++];
    error = st_Config_buildProfile(self,
        profile,
        profile_json);
    if (error)
      return error;
  }
  /* TODO: Sort the profiles by name */

  /* Retrieve the default profile */
  defaultProfile_json = json_object_get(root, "defaultProfile");
  if (!json_is_string(defaultProfile_json)) {
    fprintf(stderr, "Config error: defaultProfile is not a string\n");
    /* FIXME: This might not be an error if the user actually specified a
     * profile to use */
    return 1;
  }
  /* Binary search to find the default profile */
  {
    int a, b, i;
    int result;
    result = -1;
    st_Profile *defaultProfile;
    a = 0; b = self->internal->numProfiles;
    while (a < b) {
      i = (b - a) / 2 + a;
      defaultProfile = &self->internal->profiles[i];
      result = strcmp(
          json_string_value(defaultProfile_json),
          defaultProfile->name);
      if (result < 0) {
        b = i;
      } else if (result > 0) {
        a = i + 1;
      } else {
        /* result == 0 */
        break;
      }
    }
    if (result != 0 &&
        strcmp(
          json_string_value(defaultProfile_json),
          defaultProfile->name) != 0)
    {
      fprintf(stderr, "Config error: defaultProfile '%s' does not exist\n",
          json_string_value(defaultProfile_json));
      /* FIXME: This might not be an error if the user actually specified a
       * profile to use */
      return 1;
    }
    self->internal->defaultProfileIndex = i;
  }

  return 0;
}

int st_Config_buildProfile(
    st_Config *self,
    st_Profile *profile,
    json_t *profile_json)
{
  json_t *name, *fontFace, *fontSize, *antialiasFont, *brightIsBold;

  if (!json_is_object(profile_json)) {
    fprintf(stderr, "Config error: profile is not an object\n");
    return 1;
  }

  /* Get the name of this profile */
  name = json_object_get(profile_json, "name");
  if (name == NULL) {
    fprintf(stderr, "Config error: profile is missing a name\n");
    return 1;
  }
  if (!json_is_string(name)) {
    fprintf(stderr, "Config error: profile name is not a string\n");
    return 1;
  }
  profile->name = (char *)malloc(strlen(json_string_value(name)) + 1);
  strcpy(profile->name, json_string_value(name));

  /* Set the profile strings */
#define SET_PROFILE_STRING(key) \
  key = json_object_get(profile_json, #key); \
  if (key == NULL) { \
    fprintf(stderr, "Config error: profile '%s' missing value for '%s'", \
        profile->name, \
        #key); \
    return 1; \
  } \
  if (!json_is_string(key)) { \
    fprintf(stderr, \
        "Config error: value of '%s' in profile '%s' is not a string\n", \
        #key, \
        profile->name); \
    return 1; \
  } \
  profile->key = (char *)malloc(strlen(json_string_value(key)) + 1); \
  strcpy(profile->key, json_string_value(key));
  SET_PROFILE_STRING(fontFace)

  /* Set the numerical profile values */
  fontSize = json_object_get(profile_json, "fontSize");
  if (!json_is_real(fontSize)) {
    fprintf(stderr, "Config error: profile fontSize is not a number\n");
    return 1;
  }
  profile->fontSize = (float)json_real_value(fontSize);

  /* Set the profile flags */
  profile->flags = 0;
#define SET_PROFILE_FLAG(key,flag) \
  key = json_object_get(profile_json, #key); \
  if (!json_is_boolean(key)) { \
    fprintf(stderr, "Config error: profile " #key " is not a boolean\n"); \
    return 1; \
  } \
  profile->flags |= json_boolean_value(key) ? ST_PROFILE_ ## flag : 0;
  SET_PROFILE_FLAG(antialiasFont, ANTIALIAS_FONT);
  SET_PROFILE_FLAG(brightIsBold, BRIGHT_IS_BOLD);

  /* TODO: Set the profile colors */

  return 0;
}

void st_Config_destroy(
    st_Config *self)
{
  /* Free memory allocated for internal data structures */
  free(self->internal->profiles);
  free(self->internal);
  /* FIXME: Free memory occupied by strings stored in profiles */
}

const st_Profile *
st_Config_getProfile(
    const st_Config *self,
    const char *name)
{
  int a, b, i, result;
  st_Profile *profile;

  if (name == NULL)
    return st_Config_getDefaultProfile(self);

  /* Look for a profile with the given name using a binary search */
  a = 0; b = self->internal->numProfiles;
  while (a < b) {
    i = (b - a) / 2 + a;
    profile = &self->internal->profiles[i];
    result = strcmp(name, profile->name);
    if (result < 0) {
      b = i;
    } else if (result > 0) {
      a = i + 1;
    } else {
      /* Found the profile */
      break;
    }
  }
  profile = &self->internal->profiles[i];
  result = strcmp(name, profile->name);
  if (result != 0) {
    fprintf(stderr,
        "Unable to find terminal profile '%s'.\n"
        "Reverting to default profile.\n",
        name);
    return st_Config_getDefaultProfile(self);
  }
  return profile;
}

const st_Profile *
st_Config_getDefaultProfile(
    const st_Config *self)
{
  size_t i;

  i = self->internal->defaultProfileIndex;
  assert(i >= 0);
  assert(i < self->internal->numProfiles);

  return &self->internal->profiles[i];
}
