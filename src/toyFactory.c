/*
 * Copyright (c) 2017 Jonathan Glines
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

#include <dlfcn.h>
#include <errno.h>
#include <string.h>

#include <ttoy/plugin.h>
#include <ttoy/version.h>

#include "logging.h"
#include "pluginDictionary.h"

#include "toyFactory.h"

/* Private members */
ttoy_Plugin *
ttoy_ToyFactory_getPlugin(
    ttoy_ToyFactory *self,
    const char *name);

struct ttoy_ToyFactory_Internal {
  ttoy_PluginDictionary plugins;
  const char *pluginPath;
};

#define INIT_SIZE_PLUGINS 4

void ttoy_ToyFactory_init(
    ttoy_ToyFactory *self,
    const char *pluginPath)
{
  /* Initialize memory for internal data structures */
  self->internal = (struct ttoy_ToyFactory_Internal *)malloc(
      sizeof(struct ttoy_ToyFactory_Internal));
  ttoy_PluginDictionary_init(&self->internal->plugins);
  self->internal->pluginPath = malloc(strlen(pluginPath) + 1);
  strcpy((char *)self->internal->pluginPath, pluginPath);
}

void ttoy_ToyFactory_destroy(
    ttoy_ToyFactory *self)
{
  /* Destroy all held plugins */
  for (size_t i = 0
      ; i < ttoy_PluginDictionary_size(&self->internal->plugins)
      ; ++i)
  {
    ttoy_Plugin_destroy(
        ttoy_PluginDictionary_getValueAtIndex(
          &self->internal->plugins, i));
  }
  ttoy_PluginDictionary_destroy(&self->internal->plugins);
  /* Free internal data structure memory */
  free((char *)self->internal->pluginPath);
  free(self->internal);
}

void ttoy_ToyFactory_setPluginPath(
    ttoy_ToyFactory *self,
    const char *path)
{
  free((char *)self->internal->pluginPath);
  self->internal->pluginPath = (const char *)malloc(strlen(path) + 1);
  strcpy((char *)self->internal->pluginPath, path);
}

int comp_plugins(ttoy_Plugin **a, ttoy_Plugin **b) {
  return strcmp((*a)->name, (*b)->name);
}

ttoy_ErrorCode
ttoy_ToyFactory_registerPlugin(
    ttoy_ToyFactory *self,
    const char *name,
    const char *dlPath)
{
  void *dl;
  const uint32_t *ttoyVersion;
  const char *oldDlPath;
  ttoy_Plugin *plugin;
  const ttoy_Plugin_Attributes *pluginAttributes;
  const ttoy_Plugin_Dispatch *pluginDispatch;
  const ttoy_BackgroundToy_Attributes *backgroundToyAttributes;
  const ttoy_BackgroundToy_Dispatch *backgroundToyDispatch;
  const ttoy_TextToy_Attributes *textToyAttributes;
  const ttoy_TextToy_Dispatch *textToyDispatch;

  /* Check for an existing plugin with the same name */
  if (ttoy_ToyFactory_getPlugin(self, name) != NULL) {
    return TTOY_ERROR_DUPLICATE_PLUGIN_NAME;
  }

  /* If dlPath is not a relative or absolute path, we look for our plugin in
   * the system-wide plugin directory */
  if (strchr(dlPath, '/') == NULL) {
    /* Prepend the system-wide plugin path */
    oldDlPath = dlPath;
    /* FIXME: This memory leak is gross */
    dlPath = (const char *)malloc(
        strlen(self->internal->pluginPath) + strlen(dlPath) + 1);
    strcpy((char *)dlPath, self->internal->pluginPath);
    strcpy((char *)&dlPath[strlen(self->internal->pluginPath)], oldDlPath);
  }

  /* Open the dynamic library */
  dl = dlopen(
      dlPath,  /* filename */
      RTLD_LAZY  /* flag */
      );
  if (dl == NULL) {
    TTOY_LOG_ERROR("Plugin '%s' at path '%s' could not be loaded: %s",
        name,
        dlPath,
        dlerror());
    return TTOY_ERROR_PLUGIN_DL_FAILED_TO_LOAD;
  }

  /* Get the symbols we need from the dynamic library */
#define GET_REQUIRED_SYMBOL(SYMBOL,TYPE,OUTPUT) \
  OUTPUT = (TYPE *)dlsym(dl, #SYMBOL); \
  if (OUTPUT == NULL) { \
    TTOY_LOG_ERROR("Plugin '%s' missing symbol '%s'", \
    name, \
    #SYMBOL); \
    return TTOY_ERROR_PLUGIN_MISSING_SYMBOL; \
  }
  GET_REQUIRED_SYMBOL(
      ttoy_version,
      const uint32_t,
      ttoyVersion);

  /* Check the ttoy version this plugin was built against */
  if (*ttoyVersion != TTOY_VERSION) {
    return TTOY_ERROR_PLUGIN_VERSION_MISMATCH;
  }

  GET_REQUIRED_SYMBOL(
      TTOY_PLUGIN_ATTRIBUTES,
      const ttoy_Plugin_Attributes,
      pluginAttributes);
  GET_REQUIRED_SYMBOL(
      TTOY_PLUGIN_DISPATCH,
      const ttoy_Plugin_Dispatch,
      pluginDispatch);

  /* Get background toy symbols if needed */
  if (pluginAttributes->toyTypes & TTOY_TOY_TYPE_BACKGROUND) {
    GET_REQUIRED_SYMBOL(
        TTOY_BACKGROUND_TOY_ATTRIBUTES,
        const ttoy_BackgroundToy_Attributes,
        backgroundToyAttributes);
    GET_REQUIRED_SYMBOL(
        TTOY_BACKGROUND_TOY_DISPATCH,
        const ttoy_BackgroundToy_Dispatch,
        backgroundToyDispatch);
  } else {
    backgroundToyAttributes = NULL;
    backgroundToyDispatch = NULL;
  }

  /* Get text toy symbols if needed */
  if (pluginAttributes->toyTypes & TTOY_TOY_TYPE_TEXT) {
    GET_REQUIRED_SYMBOL(
        TTOY_TEXT_TOY_ATTRIBUTES,
        const ttoy_TextToy_Attributes,
        textToyAttributes);
    GET_REQUIRED_SYMBOL(
        TTOY_TEXT_TOY_DISPATCH,
        const ttoy_TextToy_Dispatch,
        textToyDispatch);
  } else {
    textToyAttributes = NULL;
    textToyDispatch = NULL;
  }

  /* Initialize and insert the new plugin */
  plugin = (ttoy_Plugin *)malloc(pluginAttributes->size);
  ttoy_Plugin_init(plugin,
      name,  /* name */
      pluginDispatch,  /* dispatch */
      backgroundToyAttributes,  /* backgroundToyAttributes */
      backgroundToyDispatch,  /* backgroundToyDispatch */
      textToyAttributes,  /* textToyAttributes */
      textToyDispatch  /* textToyDispatch */
      );
  pluginDispatch->init(plugin, name);
  ttoy_PluginDictionary_insert(&self->internal->plugins,
      name,  /* key */
      plugin  /* value */
      );

  return TTOY_NO_ERROR;
}

ttoy_PluginDictionary *
ttoy_ToyFactory_getPlugins(
    ttoy_ToyFactory *self)
{
  return &self->internal->plugins;
}

ttoy_Plugin *
ttoy_ToyFactory_getPlugin(
    ttoy_ToyFactory *self,
    const char *name)
{
  return ttoy_PluginDictionary_getValue(
      &self->internal->plugins,
      name);
}

ttoy_ErrorCode
ttoy_ToyFactory_buildBackgroundToy(
    ttoy_ToyFactory *self,
    const char *pluginName,
    const char *toyName,
    json_t *config,
    ttoy_BackgroundToy **toy)
{
  ttoy_Plugin *plugin;
  const ttoy_BackgroundToy_Attributes *toyAttributes;
  const ttoy_BackgroundToy_Dispatch *toyDispatch;

  /* Find the plugin with the given name */
  plugin = ttoy_ToyFactory_getPlugin(self,
      pluginName);
  if (plugin == NULL) {
    return TTOY_ERROR_PLUGIN_NOT_FOUND;
  }
  toyAttributes = ttoy_Plugin_getBackgroundToyAttributes(plugin);
  toyDispatch = ttoy_Plugin_getBackgroundToyDispatch(plugin);

  /* Allocate memory for the toy */
  *toy = (ttoy_BackgroundToy *)malloc(toyAttributes->size);
  /* Initialize the toy */
  ttoy_BackgroundToy_init(*toy,
      toyName,  /* name */
      toyDispatch  /* dispatch */
      );
  /* Call the virtual init method */
  toyDispatch->init(*toy,
      toyName,  /* name */
      config  /* config */
      );

  return TTOY_NO_ERROR;
}

ttoy_ErrorCode
ttoy_ToyFactory_buildTextToy(
    ttoy_ToyFactory *self,
    const char *pluginName,
    const char *toyName,
    json_t *config,
    ttoy_TextToy **toy)
{
  ttoy_Plugin *plugin;
  const ttoy_TextToy_Attributes *toyAttributes;
  const ttoy_TextToy_Dispatch *toyDispatch;

  /* Find the plugin with the given name */
  plugin = ttoy_ToyFactory_getPlugin(self,
      pluginName);
  if (plugin == NULL) {
    return TTOY_ERROR_PLUGIN_NOT_FOUND;
  }
  toyAttributes = ttoy_Plugin_getTextToyAttributes(plugin);
  toyDispatch = ttoy_Plugin_getTextToyDispatch(plugin);

  /* Allocate memory for the toy */
  *toy = (ttoy_TextToy *)malloc(toyAttributes->size);
  /* Initialize the toy */
  ttoy_TextToy_init(*toy,
      toyName,  /* name */
      toyDispatch  /* dispatch */
      );
  /* Call the virtual init method */
  toyDispatch->init(*toy,
      toyName,  /* name */
      config  /* config */
      );

  return TTOY_NO_ERROR;
}
