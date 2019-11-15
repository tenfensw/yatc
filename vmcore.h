//
// Yatc - Yet Another Tcl Clone
// An embeddable modern scripting language with Tcl-like syntax
// Copyright (C) Tim K/RoverAMD 2019 <timprogrammer@rambler.ru>.
// 
// vmcore.h - Yatc interpreter type and function declarations
//

#ifndef YATC_CORE
#define YATC_CORE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cext.h"
#include "vmcommon.h"
#include "vmexpr.h"
#include "vmdl.h"

struct YatcInterpreter_s;
typedef struct YatcInterpreter_s YatcInterpreter;

struct YatcInterpreterResult_s {
  unsigned success;
  unsigned line;
  char* description;
  char* lineReal;
  char** backtrace;
  void* additionalData;
};
typedef struct YatcInterpreterResult_s YatcInterpreterResult;

YatcInterpreter* yatc_interpreter_create(YatcVariable** context);
YatcInterpreterResult* yatc_interpreter_exec_(YatcInterpreter* interp, const char* line, const char* fnCallable, unsigned lineCallable);
#define yatc_interpreter_exec(interp, line) yatc_interpreter_exec_(interp, line, __FILE__, __LINE__)
void yatc_interpreter_goodbye(YatcInterpreter* interp);
void yatc_interpreter_register(YatcVariable* vr);
unsigned yatc_vector_indeed(const char* vc);
unsigned yatc_vector_length(const char* vc);
char** yatc_vector_convert(const char* vc);

#endif
