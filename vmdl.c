#include "vmdl.h"

#ifdef YATC_NODL

void* yatc_dl_dlopen(const char* fn, const int flag) {
  (void)(fn);
  (void)(flag);
  return NULL;
}

void yatc_dl_dlclose(void* handle) {
  (void)(handle);
  return;
}

void yatc_dl_contextGoodbye(YatcImportedCSymbol** csbl) {
  (void)(csbl);
  return;
}
  
YatcImportedCSymbol* yatc_dl_import(void* handle, const char* name, YatcImportedCSymbol** where) {
  (void)(handle);
  (void)(name);
  (void)(where);
  return NULL;
}

YatcImportedCSymbol** yatc_dl_allocContext(unsigned max) {
  (void)(max);
  return NULL;
}

YatcImportedCSymbol* yatc_dl_contextGet(YatcImportedCSymbol** where, const char* name) {
  (void)(where);
  (void)(name);
  return NULL;
}

#else

void* yatc_dl_dlopen(const char* fn, const int flag) {
  if (!fn)
    return NULL;
  return dlopen(fn, flag);
}

void yatc_dl_dlclose(void* handle) {
  if (!handle)
    return;
  dlclose(handle);
}

YatcImportedCSymbol* yatc_dl_import(void* handle, const char* name, YatcImportedCSymbol** where) {
  fprintf(stderr, "Called yatc_dl_import(<%p>, '%s', <%p>)\n", handle, name, where);
  if (!handle || !name || !where)
    return NULL;
  unsigned index = 0;
  while (1) {
    YatcImportedCSymbol* itm = where[index];
    if (!itm)
      break;
    else if (itm && itm->name && strcmp(itm->name, name) == 0)
      return itm;
    else
      index += 1;
  }
  where[index] = malloc(sizeof(YatcImportedCSymbol));
  where[index]->name = calloc(strlen(name) + 1, sizeof(char));
  where[index]->relatedHandle = handle;
  *(void **)(&where[index]->mem) = dlsym(handle, name);
  fprintf(stderr, "Allocated <%p>[%d] (name = '%s', handle = <%p>, mem = <%p>)\n", where, index, where[index]->name, where[index]->relatedHandle, where[index]->mem);
  return where[index];
}

void yatc_dl_contextGoodbye(YatcImportedCSymbol** csbl) {
  if (!csbl)
    return;
  unsigned index = 0;
  void* lastClosedHandle = NULL;
  while (1) {
    YatcImportedCSymbol* itm = csbl[index];
    if (!itm)
      break;
    else {
      if (lastClosedHandle != itm->relatedHandle) {
	lastClosedHandle = itm->relatedHandle;
	yatc_dl_dlclose(itm->relatedHandle);
      }
      itm->relatedHandle = NULL;
      free(itm->name);
      itm->name = NULL;
      itm->mem = NULL;
      free(itm);
      csbl[index] = NULL;
    }
    index += 1;
  }
  free(csbl);
}

YatcImportedCSymbol** yatc_dl_allocContext(unsigned max) {
  if (max < 2)
    return NULL;
  YatcImportedCSymbol** ctx = calloc(max + 1, sizeof(YatcImportedCSymbol*));
  return ctx;
}

YatcImportedCSymbol* yatc_dl_contextGet(YatcImportedCSymbol** where, const char* name) {
  fprintf(stderr, "Called yatc_dl_contextGet(<%p>, '%s')\n", where, name);
  if (!name || !where)
    return NULL;
  unsigned index = 0;
  while (1) {
    YatcImportedCSymbol* itm = where[index];
    if (!itm)
      break;
    else if (itm && itm->name && strcmp(itm->name, name) == 0)
      return itm;
    else
      index += 1;
  }
  return NULL;
}

#endif