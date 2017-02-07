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

#include <shelltoy/plugin.h>
#include <shelltoy/version.h>

#include "logging.h"
#include "pluginDictionary.h"

#include "toyFactory.h"

/* Private members */
st_Plugin *
st_ToyFactory_getPlugin(
    st_ToyFactory *self,
    const char *name);

struct st_ToyFactory_Internal {
  st_PluginDictionary plugins;
  const char *pluginPath;
};

#define INIT_SIZE_PLUGINS 4

void st_ToyFactory_init(
    st_ToyFactory *self,
    const char *pluginPath)
{
  /* Initialize memory for internal data structures */
  self->internal = (struct st_ToyFactory_Internal *)malloc(
      sizeof(struct st_ToyFactory_Internal));
  st_PluginDictionary_init(&self->internal->plugins);
  self->internal->pluginPath = malloc(strlen(pluginPath) + 1);
  strcpy((char *)self->internal->pluginPath, pluginPath);
}

void st_ToyFactory_destroy(
    st_ToyFactory *self)
{
  /* Destroy all held plugins */
  for (size_t i = 0
      ; i < st_PluginDictionary_size(&self->internal->plugins)
      ; ++i)
  {
    st_Plugin_destroy(
        st_PluginDictionary_getValueAtIndex(
          &self->internal->plugins, i));
  }
  st_PluginDictionary_destroy(&self->internal->plugins);
  /* Free internal data structure memory */
  free((char *)self->internal->pluginPath);
  free(self->internal);
}

void st_ToyFactory_setPluginPath(
    st_ToyFactory *self,
    const char *path)
{
  free((char *)self->internal->pluginPath);
  self->internal->pluginPath = (const char *)malloc(strlen(path) + 1);
  strcpy((char *)self->internal->pluginPath, path);
}

int comp_plugins(st_Plugin **a, st_Plugin **b) {
  return strcmp((*a)->name, (*b)->name);
}

st_ErrorCode
st_ToyFactory_registerPlugin(
    st_ToyFactory *self,
    const char *name,
    const char *dlPath)
{
  void *dl;
  const uint32_t *shelltoyVersion;
  const char *oldDlPath;
  st_Plugin *plugin;

  /* Check for an existing plugin with the same name */
  if (st_ToyFactory_getPlugin(self, name) != NULL) {
    return ST_ERROR_DUPLICATE_PLUGIN_NAME;
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
    ST_LOG_ERROR("Plugin '%s' at path '%s' could not be loaded: %s",
        name,
        dlPath,
        dlerror());
    return ST_ERROR_PLUGIN_DL_FAILED_TO_LOAD;
  }

  /* Get the symbols we need from the dynamic library */
#define GET_REQUIRED_SYMBOL(SYMBOL,TYPE,OUTPUT) \
  OUTPUT = (TYPE *)dlsym(dl, #SYMBOL); \
  if (OUTPUT == NULL) { \
    ST_LOG_ERROR("Plugin '%s' missing symbol '%s'", \
    name, \
    #SYMBOL); \
    return ST_ERROR_PLUGIN_MISSING_SYMBOL; \
  }
  GET_REQUIRED_SYMBOL(
      shelltoy_version,
      const uint32_t,
      shelltoyVersion);

  /* Check the Shelltoy version this plugin was built against */
  if (*shelltoyVersion != SHELLTOY_VERSION) {
    return ST_ERROR_PLUGIN_VERSION_MISMATCH;
  }

  const st_Plugin_Attributes *pluginAttributes;
  GET_REQUIRED_SYMBOL(
      SHELLTOY_PLUGIN_ATTRIBUTES,
      const st_Plugin_Attributes,
      pluginAttributes);
  const st_Plugin_Dispatch *pluginDispatch;
  GET_REQUIRED_SYMBOL(
      SHELLTOY_PLUGIN_DISPATCH,
      const st_Plugin_Dispatch,
      pluginDispatch);
  const st_BackgroundToy_Attributes *backgroundToyAttributes;
  GET_REQUIRED_SYMBOL(
      SHELLTOY_BACKGROUND_TOY_ATTRIBUTES,
      const st_BackgroundToy_Attributes,
      backgroundToyAttributes);
  const st_BackgroundToy_Dispatch *backgroundToyDispatch;
  GET_REQUIRED_SYMBOL(
      SHELLTOY_BACKGROUND_TOY_DISPATCH,
      const st_BackgroundToy_Dispatch,
      backgroundToyDispatch);

  /* Initialize and insert the new plugin */
  plugin = (st_Plugin *)malloc(pluginAttributes->size);
  st_Plugin_init(plugin,
      name,  /* name */
      pluginDispatch,  /* dispatch */
      backgroundToyAttributes,  /* backgroundToyAttributes */
      backgroundToyDispatch  /* backgroundToyDispatch */
      );
  pluginDispatch->init(plugin, name);
  st_PluginDictionary_insert(&self->internal->plugins,
      name,  /* key */
      plugin  /* value */
      );

  return ST_NO_ERROR;
}

st_PluginDictionary *
st_ToyFactory_getPlugins(
    st_ToyFactory *self)
{
  return &self->internal->plugins;
}

st_Plugin *
st_ToyFactory_getPlugin(
    st_ToyFactory *self,
    const char *name)
{
  return st_PluginDictionary_getValue(
      &self->internal->plugins,
      name);
}

st_ErrorCode
st_ToyFactory_buildBackgroundToy(
    st_ToyFactory *self,
    const char *pluginName,
    const char *toyName,
    json_t *config,
    st_BackgroundToy **toy)
{
  st_Plugin *plugin;
  const st_BackgroundToy_Attributes *toyAttributes;
  const st_BackgroundToy_Dispatch *toyDispatch;

  /* Find the plugin with the given name */
  plugin = st_ToyFactory_getPlugin(self,
      pluginName);
  if (plugin == NULL) {
    return ST_ERROR_PLUGIN_NOT_FOUND;
  }
  toyAttributes = st_Plugin_getBackgroundToyAttributes(plugin);
  toyDispatch = st_Plugin_getBackgroundToyDispatch(plugin);

  /* Allocate memory for the toy */
  *toy = (st_BackgroundToy *)malloc(toyAttributes->size);
  /* Initialize the toy */
  st_BackgroundToy_init(*toy,
      toyName,  /* name */
      toyDispatch  /* dispatch */
      );
  /* Call the virtual init method */
  toyDispatch->init(*toy,
      toyName,  /* name */
      config  /* config */
      );

  return ST_NO_ERROR;
}
