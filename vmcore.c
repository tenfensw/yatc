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
  for (unsigned i = 0; i < yatc_csarray_length(lineSplit); i++) {
    char* line = lineSplit[i];
    if (strlen(line) >= 1 && line[0] != '#') {
      char** lineReallySplit = yatc_cstring_split(line, ' ');
      char* firstCommand = lineReallySplit[0];
      if (strcmp(firstCommand, "puts") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Please specify at least what to output.");
	yatc_io_simplestOutput(lineReallySplit[1]);
      }
    }
  }
  YatcInterpreterResult* result = malloc(sizeof(YatcInterpreterResult));
  result->success = 1;
  result->description = malloc(sizeof(char));
  result->description[0] = '\0';
  result->line = 0;
  return result;
}

void yatc_interpreter_goodbye(YatcInterpreter* interp) {
  if (!interp)
    return;
  yatc_context_goodbye(interp->context);
  free(interp);
}

void yatc_interpreter_register(YatcVariable* vr);