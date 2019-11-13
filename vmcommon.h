#ifndef YATC_VMCOMMON
#define YATC_VMCOMMON

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define YATC_STRING_MINALLOC 256

enum YatcCommonType_e { YInteger = 0, YString = 1, YVector = 2, YFunction = 3, YSomething = 4 };
typedef enum YatcCommonType_e YatcCommonType;

struct YatcVariable_s;
typedef struct YatcVariable_s YatcVariable;

YatcVariable* yatc_variable_create(const char* name, YatcCommonType type, void* mem);
void yatc_variable_set(YatcVariable* vr, void* mem);
YatcCommonType yatc_variable_get_type(YatcVariable* vr);
YatcVariable** yatc_context_create(unsigned max);
void yatc_context_register(YatcVariable** context, YatcVariable* vr);
unsigned yatc_context_has(YatcVariable** context, const char* name);
YatcVariable* yatc_context_get(YatcVariable** context, const char* name);
unsigned yatc_context_length(YatcVariable** context);
void yatc_context_fprintf(FILE* where, YatcVariable** context);
void yatc_context_goodbye(YatcVariable** context);

#endif