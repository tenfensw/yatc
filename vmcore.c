#include "vmcore.h"
#include "vmio.h"

#define NOTHING_WILL_BE_DONE_SORRY "Internal interpreter error, please investigate."

struct YatcInterpreter_s {
  YatcVariable** context;
  unsigned scope;
  unsigned supportSubst;
};
typedef struct YatcInterpreter_s YatcInterpreter;

unsigned yatc_interpreter_isCrucialChar(const char tkn) {
  return (tkn == '(' || tkn == ')' || tkn == '\0' || tkn == '\n' || isspace(tkn) != 0 || ispunct(tkn) != 0);
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
    if (character == '$' && insideBrackets < 1) {
      insideName = 1;
      free(nameBuf);
      nameBuf = calloc(strlen(line) - i + 1, sizeof(char));
    } else if ((character == '{' || character == '}') && !blocked) {
      insideName = 0;
      insideBrackets += 1;
      if (character == '}')
	insideBrackets -= 2;
      finalizedLine[strlen(finalizedLine)] = character;
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
  interp->supportSubst = 1;
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

char* yatc_interpreter_execInner(YatcInterpreter* interp, const char* line) {
  if (!line)
    return NULL;
  char* result = calloc(strlen(line) * 5, sizeof(char));
  if (yatc_cstring_howMany(line, '[') < 1) {
    free(result);
    interp->supportSubst = 0;
    YatcInterpreterResult* rs = yatc_interpreter_exec(interp, line);
    interp->supportSubst = 1;
    if (!rs || !rs->additionalData)
      return NULL;
    YatcVariable* vb = rs->additionalData;
    if (yatc_variable_get_type(vb) == YString)
      return yatc_variable_get(vb);
    else if (yatc_variable_get_type(vb) == YInteger) {
      char* minBuf = calloc(10, sizeof(char));
      sprintf(minBuf, "%d", *(int*)(yatc_variable_get(vb)));
      return minBuf;
    }
    free(vb);
    return NULL;
  }
  char* bufForLine = calloc(strlen(line), sizeof(char));
  unsigned insideBrackets = 0;
  for (unsigned i = 0; i < strlen(line); i++) {
    char character = line[i];
    if (character == '[')
     insideBrackets += 1;
    else if (character == ']') {
      insideBrackets -= 1;
      if (insideBrackets == 0) {
	char* toAppend = yatc_interpreter_execInner(interp, bufForLine);
	free(bufForLine);
	if (!toAppend) {
	  toAppend = calloc(3, sizeof(char));
	  toAppend[0] = '"';
	  toAppend[1] = '"';
	}
	strcat(result, toAppend);
	free(toAppend);
	bufForLine = calloc(strlen(line), sizeof(char));
      }
    } else if (insideBrackets >= 1)
      bufForLine[strlen(bufForLine)] = character;
    else
      result[strlen(result)] = character;
  }
  free(bufForLine);
  bufForLine = NULL;
  return result;
}

YatcInterpreterResult* yatc_interpreter_exec(YatcInterpreter* interp, const char* line) {
  if (!interp || !line) {
    //fprintf(stderr, "Interpreter <%p> while parsing line '%s' <%p>\n", interp, line, line);
    return yatc_interpreter_makeAnException(0, NOTHING_WILL_BE_DONE_SORRY);
  }
  char** lineSplit = yatc_cstring_banalSplit(line, '\n');
  //yatc_csarray_fprintf(stderr, lineSplit);
  if (!lineSplit)
    return yatc_interpreter_makeAnException(0, NOTHING_WILL_BE_DONE_SORRY);
  YatcVariable* lastData = NULL;
  for (unsigned i = 0; i < yatc_csarray_length(lineSplit); i++) {
    char* lineOrig = lineSplit[i];
    //fprintf(stderr, "lineOrig = '%s'\n", lineOrig);
    char* lineUnv = yatc_interpreter_unvars(lineOrig, interp->context, interp->scope);
    //fprintf(stderr, "lineUnv = '%s'\n", lineUnv);
    char* lineTrim = yatc_cstring_trim(lineUnv);
    free(lineUnv);
    char* line = NULL;
    if (interp->supportSubst) {
      line = yatc_interpreter_execInner(interp, lineTrim);
    } else {
      line = calloc(strlen(lineTrim) + 1, sizeof(char));
      strcpy(line, lineTrim);
    }
    free(lineTrim);
    if (line && strlen(line) >= 1 && line[0] != '#') {
      if (lastData) {
	free(lastData);
	lastData = NULL;
      }
      char** lineReallySplit = yatc_cstring_split(line, ' ');
      //yatc_csarray_fprintf(stderr, lineReallySplit);
      char* firstCommand = lineReallySplit[0];
      if (strcmp(firstCommand, "puts") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Please specify at least what to output.");
	if (yatc_csarray_length(lineReallySplit) < 3)
	  yatc_io_simplestOutput(lineReallySplit[1]);
	else {
	  char* where = yatc_cstring_transformCase(lineReallySplit[1], YATC_STRING_LOWERCASE);
	  if (!where) {
	    where = calloc(8, sizeof(char));
	    strcpy(where, "stdout");
	  }
	  char* what = lineReallySplit[2];
	  if (strcmp(where, "stderr") == 0)
	    yatc_io_errorOutput(what);
	  else if (strcmp(where, "stdout") == 0)
	    yatc_io_simplestOutput(what);
	  else if (!yatc_io_fileOutput(where, what))
	    return yatc_interpreter_makeAnException(i, "Failed to flush the contents of the output file or stream.");
	}
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
	  //yatc_context_fprintf(stderr, interp->context);
	  if (strcmp(firstCommand, "const") == 0)
	    yatc_variable_makeConstant(theVariable);
	}
      } else if (strcmp(firstCommand, "expr") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Please specify the mathematical equation that you would like to evaluate.");
	char* origExpr = yatc_interpreter_unvars(lineReallySplit[1], interp->context, interp->scope);
	unsigned isSuccess = 0;
	int resulting = yatc_expressions_feedMath(origExpr, &isSuccess);
	if (!isSuccess)
	  return yatc_interpreter_makeAnException(i, "Mathematical expression evaluation error.");
	int* resultingAlc = malloc(sizeof(int));
	*resultingAlc = resulting;
	YatcVariable* theVariable = yatc_variable_create("", YInteger, resultingAlc, interp->scope);
	lastData = theVariable;
      } else if (strcmp(firstCommand, "prompt") == 0) {
	char* what = NULL;
	if (yatc_csarray_length(lineReallySplit) >= 2)
	  what = lineReallySplit[1];
	char* buf = yatc_io_prompt(what);
	YatcVariable* vb = yatc_variable_create("", YString, buf, interp->scope);
	lastData = vb;
      } else if (strcmp(firstCommand, "read") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Please specify a path to the file or a URL from which the data shall be read.");
	if (yatc_io_fileExists(lineReallySplit[1])) {
	  char* buf = yatc_io_readAll(lineReallySplit[1]);
	  if (!buf)
	    return yatc_interpreter_makeAnException(i, "File reading error, do you have permissions to read the file?");
	  lastData = yatc_variable_create("", YString, buf, interp->scope);
	} else
	  return yatc_interpreter_makeAnException(i, "File does not exist.");
      } else if (strcmp(firstCommand, "foreach") == 0} {
	if (yatc_csarray_length(lineReallySplit) < 4)
	  return yatc_interpreter_makeAnException(i, "Please specify the name of the iterator, the iterable and the code to execute.");
	// TODO
      } else if (strcmp(firstCommand, "incr") == 0 || strcmp(firstCommand, "decr") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Please specify at least the name of the variable that you want to increment.");
	char* coefOrig = lineReallySplit[2];
	if (!coefOrig) {
	  coefOrig = malloc(sizeof(char) * 2);
	  coefOrig[0] = '0';
	  coefOrig[1] = '\0';
	}
	int coef = atoi(coefOrig);
	for (unsigned i = 1; i < yatc_csarray_length(lineReallySplit); i++) {
	  char* vn = lineReallySplit[i];
	  if (!yatc_context_has(interp->context, vn, interp->scope))
	    return yatc_interpreter_makeAnException(i, "One or more variables are not declared in this scope.");
	  YatcVariable* vbl = yatc_context_get(interp->context, vn, interp->scope);
	  if (yatc_variable_get_type(vbl) != YInteger)
	    return yatc_interpreter_makeAnException(i, "Strings cannot be incremented.");
	  int* mem = yatc_variable_get(vbl);
	  if (strcmp(firstCommand, "decr") == 0)
	    *(mem) -= coef;
	  else
	    *(mem) += coef;
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