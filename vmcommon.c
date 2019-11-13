#include "vmcommon.h"

struct YatcVariable_s {
  char* name;
  YatcCommonType type;
  void* mem;
};
typedef struct YatcVariable_s YatcVariable;

YatcVariable* yatc_variable_create(const char* name, YatcCommonType type, void* mem) {
  if (!name || type == YSomething)
    return NULL;
  YatcVariable* result = malloc(sizeof(YatcVariable));
  result->type = type;
  result->name = calloc(strlen(name) + 1, sizeof(char));
  strcpy(result->name, name);
  result->mem = mem;
  return result;
}

void yatc_variable_set(YatcVariable* vr, void* mem) {
  if (!vr)
    return;
  vr->mem = mem;
}

YatcCommonType yatc_variable_get_type(YatcVariable* vr) {
  if (!vr)
    return YSomething;
  return vr->type;
}

YatcVariable** yatc_context_create(unsigned max) {
  if (max < 1)
    return NULL;
  YatcVariable** result = calloc(max + 1, sizeof(YatcVariable*));
  return result;
}

unsigned yatc_context_length(YatcVariable** context) {
  if (!context)
    return 0;
  unsigned count = 0;
  while (1) {
    if (context[count])
      count += 1;
    else
      break;
  }
  return count;
}

void yatc_context_register(YatcVariable** context, YatcVariable* vr) {
  if (!context || !vr)
    return;
  context[yatc_context_length(context)] = vr;
}

unsigned yatc_context_has(YatcVariable** context, const char* name) {
  return (yatc_context_get(context, name) != NULL);
}

YatcVariable* yatc_context_get(YatcVariable** context, const char* name) {
  if (!context || !name)
    return NULL;
  YatcVariable* result = NULL;
  for (unsigned i = 0; i < yatc_context_length(context); i++) {
    if (context[i] && context[i]->name && strcmp(context[i]->name, name) == 0)
      result = context[i];
  }
  return result;
}

const char* yatc_type_stringify(YatcCommonType tp) {
  if (tp == YString)
    return "YString";
  else if (tp == YInteger)
    return "YInteger (32-bit signed)";
  else if (tp == YVector)
    return "YVector (array-based list implementation)";
  else if (tp == YFunction)
    return "YFunction (method with infinite arguments)";
  else
    return "YSomething (null)";
}

void yatc_context_goodbye(YatcVariable** context) {
  if (!context)
    return;
  for (unsigned i = 0; i < yatc_context_length(context); i++) {
    free(context[i]);
    context[i] = NULL;
  }
  free(context);
}

void yatc_context_fprintf(FILE* where, YatcVariable** context) {
  if (!context)
    return;
  fprintf(where, "%d total\n", yatc_context_length(context));
  for (unsigned i = 0; i < yatc_context_length(context); i++) {
    YatcVariable* cv = context[i];
    if (cv && cv->name) {
      fprintf(where, "[%d] %s <%p> <=> %s <=> ", (i + 1), cv->name, cv, yatc_type_stringify(cv->type));
      if (cv->mem && cv->type != YSomething) {
	if (cv->type == YString)
	  fprintf(where, "'%s' ", cv->mem);
	else if (cv->type == YInteger)
	  fprintf(where, "%d ", *(double*)(cv->mem));
	fprintf(where, "<%p>\n", cv->mem);
      } else
	fprintf(where, "(null)\n");
    } else
      fprintf(where, "[%d] (null)\n", (i + 1));
  }
}