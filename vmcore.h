#ifndef YATC_CORE
#define YATC_CORE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cext.h"
#include "vmcommon.h"
#include "vmexpr.h"

struct YatcInterpreter_s;
typedef struct YatcInterpreter_s YatcInterpreter;

struct YatcInterpreterResult_s {
  unsigned success;
  unsigned line;
  char* description;
  void* additionalData;
};
typedef struct YatcInterpreterResult_s YatcInterpreterResult;

YatcInterpreter* yatc_interpreter_create(YatcVariable** context);
YatcInterpreterResult* yatc_interpreter_exec(YatcInterpreter* interp, const char* line);
void yatc_interpreter_goodbye(YatcInterpreter* interp);
void yatc_interpreter_register(YatcVariable* vr);

#endif