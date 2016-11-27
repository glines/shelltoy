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
#include <jsmn.h>
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
  self->internal->sizeProfiles = ST_CONFIG_INIT_SIZE_PROFILES;
  self->internal->profiles = (st_Profile *)malloc(
      sizeof(st_Profile) * self->internal->sizeProfiles);
  self->internal->numProfiles = 0;
  self->internal->defaultProfileIndex = -1;
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
  jsmn_parser parser;
  jsmntok_t *tokens;
  size_t numTokens;
  int result;

  jsmn_init(&parser);

  /* Determine the number of tokens in this config file */
  result = jsmn_parse(&parser, config, len, NULL, 0);
  if (result < 1) {
    const char *reason;
    switch (result) {
      case JSMN_ERROR_INVAL:
        reason = "Bad token; JSON string is corrupted.";
        break;
      case JSMN_ERROR_PART:
        reason = "JSON string is too short; expecting more JSON data.";
        break;
    }
    /* FIXME: Find a JSON library with better error diagnostics */
    fprintf(stderr, "Failed to parse config file: %s.",
        reason);
  }
  tokens = (jsmntok_t *)malloc(numTokens * sizeof(jsmntok_t));
  /* Parse the config file */
  result = jsmn_parse(&parser, config, len, tokens, numTokens);
  assert(result >= 0);

  free(tokens);
}

void st_Config_destroy(
    st_Config *self)
{
  /* Free memory allocated for internal data structures */
  free(self->internal->profiles);
  free(self->internal);
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
