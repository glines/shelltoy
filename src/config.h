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

#include "error.h"
#include "profile.h"
#include <ttoy/backgroundToy.h>

struct ttoy_Config_Internal;

typedef struct ttoy_Config {
  struct ttoy_Config_Internal *internal;
  char *configFilePath;
} ttoy_Config;

void ttoy_Config_init(
    ttoy_Config *self);

void ttoy_Config_destroy(
    ttoy_Config *self);

void ttoy_Config_setPluginPath(
    ttoy_Config *self,
    const char *pluginPath);

/**
 * This routine looks for a config file in the default locations, and stores
 * that path in the config object.
 *
 * If a config file is found, then TTOY_NO_ERROR is returned and the
 * configFilePath member of the ttoy_Config object then contains a pointer to a
 * nul-terminated string representing the path to that file.
 *
 * If a config file is not found, then TTOY_ERROR_CONFIG_FILE_NOT_FOUND.
 *
 * Note that the config file is not read or parsed until
 * ttoy_Config_readConfigFile() is called at a later time.
 *
 * At the moment, the only config file location searched is
 * "$HOME/.config/ttoy/config.json".
 *
 * \sa ttoy_Config_readConfigFile()
 */
ttoy_ErrorCode ttoy_Config_findConfigFile(
    ttoy_Config *self);

/**
 * This routine copies the path from 
 *
 * Nothing is done to 
 */
ttoy_ErrorCode ttoy_Config_setConfigFilePath(
    ttoy_Config *self,
    const char *path);

/**
 * Gets the address of the profile with the given name and stores it in
 * *profile.
 *
 * If no such profile is found, then TTOY_ERROR_PROFILE_NOT_FOUND is returned and
 * *profile is set to NULL.
 */
ttoy_ErrorCode
ttoy_Config_getProfile(
    ttoy_Config *self,
    const char *name,
    ttoy_Profile **profile);

ttoy_ErrorCode
ttoy_Config_getDefaultProfile(
    ttoy_Config *self,
    ttoy_Profile **profile);

ttoy_ErrorCode
ttoy_Config_createDefaultConfigFile(
    const ttoy_Config *self);
