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

#ifndef SHELLTOY_ERROR_CODES_H_
#define SHELLTOY_ERROR_CODES_H_

ST_START_ERROR_CODES
ST_DECLARE_ERROR_CODE(
    ST_NO_ERROR,  /* code */
    "No error"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_ATLAS_GLYPH_NOT_FOUND,  /* code */
    "Could not find atlas glyph"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_CONFIG,  /* code */
    "Error in the configuration file"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_CONFIG_FAILED_TO_SERIALIZE,  /* code */
    "Failed to serialize the configuration to JSON"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_CONFIG_FILE_FORMAT,  /* code */
    "Error in the format of the configuration file"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_CONFIG_FILE_NOT_FOUND,  /* code */
    "File not found"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_CONFIG_FILE_PATH_NOT_SET,  /* code */
    "Path to configuration file not set"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_CONFIG_FILE_READ,  /* code */
    "Error reading the configuration file"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_DUPLICATE_PLUGIN_NAME,  /* code */
    "Duplicate plugin names"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_FAILED_TO_CREATE_CONFIG_FILE,  /* code */
    "Failed to create configuration file"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_FAILED_TO_CREATE_DIRECTORY,  /* code */
    "Failed to create directory"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_FAILED_TO_LOAD_FONT,  /* code */
    "Failed to load font"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_FONTCONFIG_ERROR,  /* code */
    "Fontconfig error; could not locate a font"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_FONT_GLYPH_NOT_FOUND,  /* code */
    "Could not find glyph in a font"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_FONT_NOT_FOUND,  /* code */
    "Could not find suitable font"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_FREETYPE_ERROR,  /* code */
    "FreeType error; could not load a font"  /* string */
    )
/* FIXME: FONT_NOT_FOUND and MISSING_FONT are too similar */
ST_DECLARE_ERROR_CODE(
    ST_ERROR_MISSING_FONT,  /* code */
    "Missing a font file"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_MISSING_FONT_FOR_CHARACTER_CODE,  /* code */
    "Could not font providing glyph for a character code"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_NEGATIVE_FONT_SIZE,  /* code */
    "Negative font size"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_OPENING_SHADER_FILE,  /* code */
    "Error opening a shader file"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_OUT_OF_MEMORY,  /* code */
    "Out of memory"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_PLUGIN_DL_FAILED_TO_LOAD,  /* code */
    "Failed to load dynamic library for plugin"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_PLUGIN_MISSING_SYMBOL,  /* code */
    "Dynamic library of plugin is missing a required symbol"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_PLUGIN_NOT_FOUND,  /* code */
    "Could not find plugin"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_PLUGIN_VERSION_MISMATCH,  /* code */
    "Plugin was compiled for a different version of Shelltoy"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_PROFILE_NOT_FOUND,  /* code */
    "Specified profile was not found"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_PROFILE_NO_PRIMARY_FONT,  /* code */
    "Profile has no primary font"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_SDL_ERROR,  /* code */
    "SDL error"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_SHADER_COMPILATION_FAILED,  /* code */
    "Failed to compile shader"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_SHADER_FILE_NOT_FOUND,  /* code */
    "Could not find shader file"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_SHADER_LINKING_FAILED,  /* code */
    "Failed to link shader program"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_UNKNOWN_COLOR_CODE,  /* code */
    "Unknown terminal color escape code"  /* string */
    )
ST_DECLARE_ERROR_CODE(
    ST_ERROR_UNKNOWN_SHADER_TYPE,  /* code */
    "Unknown shader type"  /* string */
    )
ST_END_ERROR_CODES

#endif
