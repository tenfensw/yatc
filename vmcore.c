#include "vmcore.h"
#include "vmio.h"

#define NOTHING_WILL_BE_DONE_SORRY "Internal interpreter error, please investigate."

struct YatcInterpreter_s {
  YatcVariable** context;
  unsigned scope;
};
typedef struct YatcInterpreter_s YatcInterpreter;

unsigned yatc_interpreter_isCrucialChar(const char tkn) {
  return (tkn == ' ' || tkn == ',' || tkn == '\t' || tkn == '(' || tkn == ')' || tkn == '\0');
}

char* yatc_interpreter_unvars(const char* line, YatcVariable** context, unsigned scope) {
  if (!line || !context)
    return NULL;
  char* nameBuf = NULL;
  char* finalizedLine = calloc(strlen(line) * 10, sizeof(char));
  unsigned insideName = 0;
  unsigned insideBrackets = 0;
  unsigned blocked = 0;
  for (unsigned i = 0; i < strlen(line) + 1; i++) {
    char character = line[i];
    if (character == '$') {
      insideName = 1;
      free(nameBuf);
      nameBuf = calloc(strlen(line) - i + 1, sizeof(char));
    } else if (yatc_interpreter_isCrucialChar(character)) {
      insideName = 0;
      if (nameBuf) {
	if (yatc_context_has(context, nameBuf, scope)) {
	  YatcVariable* vr = yatc_context_get(context, nameBuf, scope);
	  char* bufMax = calloc(strlen(line) / 2, sizeof(char));
	  void* mm = yatc_variable_get(vr);
	  if (mm) {
	    YatcCommonType tp = yatc_variable_get_type(vr);
	    if (tp == YInteger)
	      sprintf(bufMax, "%d", *(int*)(mm));
	    else if (tp == YString) {
	      free(bufMax);
	      bufMax = calloc(strlen(mm) + 1, sizeof(char));
	      strcpy(bufMax, mm);
	    } else
	      strcpy(bufMax, "0");
	  } else
	    strcpy(bufMax, "0");
	  strcat(finalizedLine, bufMax);
	  free(bufMax);
	  free(nameBuf);
	  nameBuf = NULL;
	}
      }
      finalizedLine[strlen(finalizedLine)] = character;
    } else if (character == '\\' && insideBrackets < 1) {
      insideName = 0;
      blocked = !(blocked);
      if (!blocked)
	finalizedLine[strlen(finalizedLine)] = character;
      continue;
    } else if ((character == '{' || character == '}') && !blocked) {
      insideName = 0;
      insideBrackets += 1;
      if (character == '}')
	insideBrackets -= 2;
      finalizedLine[strlen(finalizedLine)] = character;
    } else {
      if (blocked)
	blocked = 0;
      if (insideName)
	nameBuf[strlen(nameBuf)] = character;
      else
	finalizedLine[strlen(finalizedLine)] = character;
    }
  }
  free(nameBuf);
  return finalizedLine;
}

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
    //fprintf(stderr, "Interpreter <%p> while parsing line '%s' <%p>\n", interp, line, line);
    return yatc_interpreter_makeAnException(0, NOTHING_WILL_BE_DONE_SORRY);
  }
  char** lineSplit = yatc_cstring_split(line, '\n');
  if (!lineSplit)
    return yatc_interpreter_makeAnException(0, NOTHING_WILL_BE_DONE_SORRY);
  YatcVariable* lastData = NULL;
  for (unsigned i = 0; i < yatc_csarray_length(lineSplit); i++) {
    char* lineOrig = lineSplit[i];
    fprintf(stderr, "lineOrig = '%s'\n", lineOrig);
    char* lineUnv = yatc_interpreter_unvars(lineOrig, interp->context, interp->scope);
    fprintf(stderr, "lineUnv = '%s'\n", lineUnv);
    char* line = yatc_cstring_trim(lineUnv);
    free(lineUnv);
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
	//fprintf(stderr, "yatc_csarray_length(lineReallySplit) = %d\n", yatc_csarray_length(lineReallySplit));
	if (yatc_csarray_length(lineReallySplit) < 3 || (((yatc_csarray_length(lineReallySplit) - 1) % 2) != 0))
	  return yatc_interpreter_makeAnException(i, "Please specify at least one pair of arguments - the first one is the variable name, the second one is its value.");
	for (unsigned i = 1; i < (yatc_csarray_length(lineReallySplit)); i += 2) {
	  char* key = lineReallySplit[i];
	  char* value = lineReallySplit[i + 1];
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
	  if (!yatc_context_unregister(interp->context, key, interp->scope, theVariable))
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
      free(line);
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