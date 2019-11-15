//
// Yatc - Yet Another Tcl Clone
// An embeddable modern scripting language with Tcl-like syntax
// Copyright (C) Tim K/RoverAMD 2019 <timprogrammer@rambler.ru>.
// 
// vmdl.h - Yatc interpreter dynamic library loading abstraction
// layer
//

#ifndef YATC_VMDL
#define YATC_VMDL

#include <dlfcn.h>
#include <stdlib.h>
#include "vmcommon.h"

struct YatcImportedCSymbol_s {
  char* name;
  YatcVariable* (*mem)(YatcVariable**);
  void* relatedHandle;
};

typedef struct YatcImportedCSymbol_s YatcImportedCSymbol;

#ifndef RTLD_LAZY
#define RTLD_LAZY RTLD_NOW
#warning "RTLD_LAZY is not supported on this platform, RTLD_NOW will be used for now, but that might just make Yatc slower."
#endif

void* yatc_dl_dlopen(const char* fn, const int flag);
void yatc_dl_dlclose(void* handle);
YatcImportedCSymbol* yatc_dl_import(void* handle, const char* name, YatcImportedCSymbol** where);
YatcImportedCSymbol** yatc_dl_allocContext(unsigned max);
YatcImportedCSymbol* yatc_dl_contextGet(YatcImportedCSymbol** where, const char* name);
void yatc_dl_contextGoodbye(YatcImportedCSymbol** csbl);

#endif