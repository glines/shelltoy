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

#include "./common/version.h"
#include "logging.h"
#include "plugin.h"

#include "toyFactory.h"

/* Private members */
st_Plugin *
st_ToyFactory_getPlugin(
    st_ToyFactory *self,
    const char *name);

struct st_ToyFactory_Internal {
  st_Plugin **plugins;
  size_t sizePlugins, numPlugins;
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
  self->internal->plugins = (st_Plugin **)malloc(
      sizeof(st_Plugin *) * INIT_SIZE_PLUGINS);
  self->internal->sizePlugins = INIT_SIZE_PLUGINS;
  self->internal->numPlugins = 0;
  self->internal->pluginPath = malloc(strlen(pluginPath) + 1);
  strcpy((char *)self->internal->pluginPath, pluginPath);
}

void st_ToyFactory_destroy(
    st_ToyFactory *self)
{
  /* Destroy all held plugins */
  for (size_t i = 0; i < self->internal->numPlugins; ++i) {
    st_Plugin_destroy(
        self->internal->plugins[i]);
  }
  /* Free internal data structure memory */
  free(self->internal->plugins);
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
      SHELLTOY_VERSION,
      const uint32_t,
      shelltoyVersion);

  /* Check the Shelltoy version this plugin was built against */
  if (*shelltoyVersion != ST_VERSION) {
    return ST_ERROR_PLUGIN_VERSION_MISMATCH;
  }

  const st_PluginAttributes *attributes;
  GET_REQUIRED_SYMBOL(
      SHELLTOY_PLUGIN_ATTRIBUTES,
      const st_PluginAttributes,
      attributes);
  const st_PluginDispatch *dispatch;
  GET_REQUIRED_SYMBOL(
      SHELLTOY_PLUGIN_DISPATCH,
      const st_PluginDispatch,
      dispatch);

  /* Ensure we have memory allocated to store a pointer to this plugin */
  if (self->internal->numPlugins + 1 > self->internal->sizePlugins) {
    st_Plugin **newPlugins;
    newPlugins = (st_Plugin **)malloc(
        sizeof(st_Plugin *) * self->internal->sizePlugins * 2);
    memcpy(
        newPlugins,
        self->internal->plugins,
        sizeof(st_Plugin *) * self->internal->numPlugins);
    free(self->internal->plugins);
    self->internal->plugins = newPlugins;
    self->internal->sizePlugins *= 2;
  }

  /* Initialize and insert the new plugin */
  plugin = (st_Plugin *)malloc(attributes->pluginSize);
  st_Plugin_init(plugin, name);
  dispatch->init(plugin, name);
  self->internal->plugins[self->internal->numPlugins++] = plugin;

  /* Sort the plugins by name */
  qsort(
      self->internal->plugins,  /* ptr */
      self->internal->numPlugins,  /* count */
      sizeof(self->internal->plugins[0]),  /* size */
      (int (*)(const void *, const void *))comp_plugins  /* comp */
      );

  return ST_NO_ERROR;
}

st_Plugin *
st_ToyFactory_getPlugin(
    st_ToyFactory *self,
    const char *name)
{
  int a, b, i;
  if (self->internal->numPlugins == 0)
    return NULL;
  /* Find the plugin with a binary search */
  a = 0; b = self->internal->numPlugins;
  while (b - a > 0) {
    int result;
    i = (b - a) / 2 + a;
    result = strcmp(name, self->internal->plugins[i]->name);
    if (result < 0) {
      b = i;
    } else if (result > 0) {
      a = i + 1;
    } else {
      /* result == 0 */
      return self->internal->plugins[i];
    }
  }
  /* Scan for the plugin in the range a to b */
  for (i = a; i < b; ++i) {
    int result = strcmp(name, self->internal->plugins[i]->name);
    if (result == 0) {
      return self->internal->plugins[i];
    }
  }
  return NULL;
}

st_ErrorCode
st_ToyFactory_buildToy(
    st_ToyFactory *self,
    const char *pluginName,
    json_t *config,
    st_Toy *toy)
{
  st_Plugin *plugin;

  /* Find the plugin with the given name */
  plugin = st_ToyFactory_getPlugin(self,
      pluginName);
  if (plugin == NULL) {
    return ST_ERROR_PLUGIN_NOT_FOUND;
  }

  return ST_NO_ERROR;
}
