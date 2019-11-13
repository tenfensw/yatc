#include "vmcore.h"
#include "vmio.h"

#define NOTHING_WILL_BE_DONE_SORRY "Internal interpreter error, please investigate."

struct YatcInterpreter_s {
  YatcVariable** context;
  unsigned scope;
};
typedef struct YatcInterpreter_s YatcInterpreter;

YatcInterpreter* yatc_interpreter_create(YatcVariable** context) {
  YatcInterpreter* interp = malloc(sizeof(YatcInterpreter));
  interp->scope = 0;
  interp->context = calloc(1024, sizeof(YatcVariable*));
  if (context)
    yatc_context_migrate(interp->context, context);
  return interp;
}

YatcInterpreterResult* yatc_interpreter_makeAnException(unsigned line, const char* text) {
  if (!text)
    return NULL;
  YatcInterpreterResult* res = malloc(sizeof(YatcInterpreterResult));
  res->success = 0;
  res->line = line + 1;
  res->description = calloc(strlen(text) + 1, sizeof(char));
  res->additionalData = NULL;
  strcpy(res->description, text);
  return res;
}

YatcInterpreterResult* yatc_interpreter_exec(YatcInterpreter* interp, const char* line) {
  if (!interp || !line) {
    fprintf(stderr, "Interpreter <%p> while parsing line '%s' <%p>\n", interp, line, line);
    return yatc_interpreter_makeAnException(0, NOTHING_WILL_BE_DONE_SORRY);
  }
  char** lineSplit = yatc_cstring_split(line, '\n');
  if (!lineSplit)
    return yatc_interpreter_makeAnException(0, NOTHING_WILL_BE_DONE_SORRY);
  YatcVariable* lastData = NULL;
  for (unsigned i = 0; i < yatc_csarray_length(lineSplit); i++) {
    char* line = lineSplit[i];
    if (strlen(line) >= 1 && line[0] != '#') {
      if (lastData) {
	free(lastData);
	lastData = NULL;
      }
      char** lineReallySplit = yatc_cstring_split(line, ' ');
      yatc_csarray_fprintf(stderr, lineReallySplit);
      char* firstCommand = lineReallySplit[0];
      if (strcmp(firstCommand, "puts") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Please specify at least what to output.");
	yatc_io_simplestOutput(lineReallySplit[1]);
      } else if (strcmp(firstCommand, "set") == 0 || strcmp(firstCommand, "const") == 0) {
	fprintf(stderr, "yatc_csarray_length(lineReallySplit) = %d\n", yatc_csarray_length(lineReallySplit));
	if (yatc_csarray_length(lineReallySplit) < 3 || (((yatc_csarray_length(lineReallySplit) - 1) % 2) != 0))
	  return yatc_interpreter_makeAnException(i, "Please specify at least one pair of arguments - the first one is the variable name, the second one is its value.");
	for (unsigned i = 1; i < (yatc_csarray_length(lineReallySplit)); i += 2) {
	  char* key = lineReallySplit[i];
	  char* value = lineReallySplit[i + 1];
	  if (yatc_context_has(interp->context, key, interp->scope))
	    return yatc_interpreter_makeAnException(i, "Function or variable with the same name already exists in this scope.");
	  YatcInterpreterResult* whatEvalSays = yatc_interpreter_exec(interp, value);
	  if (whatEvalSays->success)
	    free(whatEvalSays->description);
	  else
	    return yatc_interpreter_makeAnException(i, whatEvalSays->description);
	  YatcVariable* theVariable = (YatcVariable*)(whatEvalSays->additionalData);
	  if (!theVariable)
	    theVariable = yatc_variable_create(key, YSomething, NULL, interp->scope);
	  else
	    yatc_variable_set_name(theVariable, key);
	  yatc_context_register(interp->context, theVariable);
	  yatc_context_fprintf(stderr, interp->context);
	  if (strcmp(firstCommand, "const") == 0)
	    yatc_variable_makeConstant(theVariable);
	}
      } else {
	int numericValue = atoi(line);
	YatcCommonType tp = YString;
	void* value = calloc(strlen(line) + 1, sizeof(char));
	strcpy(value, line);
	if (atoi(line) != 0 || strcmp(line, "0") == 0) {
	  tp = YInteger;
	  value = malloc(sizeof(int));
	  *(int*)(value) = numericValue;
	}
	YatcVariable* vb = yatc_variable_create("", tp, value, interp->scope);
	lastData = vb;
      }
      free(lineReallySplit);
    }
  }
  free(lineSplit);
  YatcInterpreterResult* result = malloc(sizeof(YatcInterpreterResult));
  result->success = 1;
  result->description = malloc(sizeof(char));
  result->description[0] = '\0';
  result->line = 0;
  result->additionalData = lastData;
  return result;
}


void yatc_interpreter_goodbye(YatcInterpreter* interp) {
  if (!interp)
    return;
  yatc_context_goodbye(interp->context);
  free(interp);
}

void yatc_interpreter_register(YatcVariable* vr);