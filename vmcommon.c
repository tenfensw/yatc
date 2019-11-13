#include "vmcommon.h"

struct YatcVariable_s {
  char* name;
  YatcCommonType type;
  void* mem;
  unsigned constant;
  unsigned scope;
};
typedef struct YatcVariable_s YatcVariable;

YatcVariable* yatc_variable_create(const char* name, YatcCommonType type, void* mem, unsigned scope) {
  if (!name)
    return NULL;
  YatcVariable* result = malloc(sizeof(YatcVariable));
  result->type = type;
  result->scope = scope;
  result->constant = 0;
  result->name = calloc(strlen(name) + 1, sizeof(char));
  strcpy(result->name, name);
  result->mem = mem;
  return result;
}

unsigned yatc_variable_set(YatcVariable* vr, void* mem) {
  if (!vr || vr->constant)
    return 0;
  vr->mem = mem;
  return 1;
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

unsigned yatc_variable_get_scope(YatcVariable* vr) {
  if (!vr)
    return 0;
  return vr->scope;
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

void yatc_variable_makeConstant(YatcVariable* vl) {
  if (!vl)
    return;
  vl->constant = 1;
}

void yatc_context_migrate(YatcVariable** c1, YatcVariable** c2) {
  if (!c1 || !c2)
    return;
  for (unsigned i = 0; i < yatc_context_length(c2); i++) {
    YatcVariable* mirror = malloc(sizeof(YatcVariable));
    YatcVariable* orig = c2[i];
    if (!orig)
      return;
    mirror->name = calloc(strlen(orig->name) + 1, sizeof(char));
    strcpy(mirror->name, orig->name);
    mirror->type = orig->type;
    mirror->scope = orig->scope;
    mirror->mem = NULL;
    if (orig->type == YString) {
      mirror->mem = calloc(strlen(orig->mem) + 1, sizeof(char));
      strcpy(mirror->mem, orig->mem);
    } else if (orig->type == YInteger) {
      mirror->mem = malloc(sizeof(int));
      *(int*)(mirror->mem) = *(int*)(orig->mem);
    }
    yatc_context_register(c1, mirror);
  }
}

void yatc_context_register(YatcVariable** context, YatcVariable* vr) {
  //fprintf(stderr, "context <%p>, vr <%p>\n", context, vr);
  if (!context || !vr)
    return;
  context[yatc_context_length(context)] = vr;
}

unsigned yatc_context_has(YatcVariable** context, const char* name, unsigned scope) {
  return (yatc_context_get(context, name, scope) != NULL);
}

YatcVariable* yatc_context_get(YatcVariable** context, const char* name, unsigned scope) {
  if (!context || !name)
    return NULL;
  YatcVariable* result = NULL;
  for (unsigned i = 0; i < yatc_context_length(context); i++) {
    if (context[i] && context[i]->name && strcmp(context[i]->name, name) == 0 && context[i]->scope == scope)
      result = context[i];
  }
  return result;
}

void* yatc_variable_get(YatcVariable* vr) {
  if (!vr)
    return NULL;
  return vr->mem;
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

unsigned yatc_context_unregister(YatcVariable** context, const char* name, unsigned scope, YatcVariable* newv) {
  if (!name || !context || !newv || !yatc_context_has(context, name, scope))
    return 0;
  for (unsigned i = 0; i < yatc_context_length(context); i++) {
    if (context[i] && context[i]->name && context[i]->scope == scope && strcmp(context[i]->name, name) == 0) {
      free(context[i]);
      context[i] = newv;
      return 1;
    }
  }
  return 0;
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

void yatc_variable_set_name(YatcVariable* vl, const char* nm) {
  if (!vl || !nm)
    return;
  free(vl->name);
  vl->name = calloc(strlen(nm) + 1, sizeof(char));
  strcpy(vl->name, nm);
}

void yatc_context_fprintf(FILE* where, YatcVariable** context) {
  if (!context)
    return;
  fprintf(where, "%d total\n", yatc_context_length(context));
  for (unsigned i = 0; i < yatc_context_length(context); i++) {
    YatcVariable* cv = context[i];
    if (cv && cv->name) {
      fprintf(where, "[%d] %s (scope #%d) <%p> <=> %s <=> ", (i + 1), cv->name, cv->scope, cv, yatc_type_stringify(cv->type));
      if (cv->mem && cv->type != YSomething) {
	if (cv->type == YString)
	  fprintf(where, "'%s' ", (char*)(cv->mem));
	else if (cv->type == YInteger)
	  fprintf(where, "%d ", *(int*)(cv->mem));
	fprintf(where, "<%p>\n", cv->mem);
      } else
	fprintf(where, "(null)\n");
    } else
      fprintf(where, "[%d] (null)\n", (i + 1));
  }
}