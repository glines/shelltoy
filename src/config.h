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

#include "error.h"
#include "profile.h"

struct st_Config_Internal;

typedef struct st_Config {
  struct st_Config_Internal *internal;
  char *configFilePath;
} st_Config;

void st_Config_init(
    st_Config *self);

void st_Config_destroy(
    st_Config *self);

/**
 * This routine looks for a config file in the default locations, and stores
 * that path in the config object.
 *
 * If a config file is found, then ST_NO_ERROR is returned and the
 * configFilePath member of the st_Config object then contains a pointer to a
 * nul-terminated string representing the path to that file.
 *
 * If a config file is not found, then ST_ERROR_CONFIG_FILE_NOT_FOUND.
 *
 * Note that the config file is not read or parsed until
 * st_Config_readConfigFile() is called at a later time.
 *
 * At the moment, the only config file location searched is
 * "$HOME/.config/shelltoy/config.json".
 *
 * \sa st_Config_readConfigFile()
 */
st_ErrorCode st_Config_findConfigFile(
    st_Config *self);

/**
 * This routine copies the path from 
 *
 * Nothing is done to 
 */
st_ErrorCode st_Config_setConfigFilePath(
    st_Config *self,
    const char *path);

/**
 * Gets the address of the profile with the given name and stores it in
 * *profile.
 *
 * If no such profile is found, then ST_ERROR_PROFILE_NOT_FOUND is returned and
 * *profile is set to NULL.
 */
st_ErrorCode
st_Config_getProfile(
    st_Config *self,
    const char *name,
    st_Profile **profile);

st_ErrorCode
st_Config_getDefaultProfile(
    st_Config *self,
    st_Profile **profile);
