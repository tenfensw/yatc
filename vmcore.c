//
// Yatc - Yet Another Tcl Clone
// An embeddable modern scripting language with Tcl-like syntax
// Copyright (C) Tim K/RoverAMD 2019 <timprogrammer@rambler.ru>.
// 
// vmcore.c - Implementation of the core interpreter business logic
//

#include "vmcore.h"
#include "vmio.h"
#include <execinfo.h>

#define NOTHING_WILL_BE_DONE_SORRY "Internal interpreter error, please investigate."

#define _ctxsize 1024
#define _vssize (_ctxsize * 2)

struct YatcInterpreter_s {
  YatcVariable** context;
  unsigned scope;
  unsigned supportSubst;
  YatcImportedCSymbol** dlContext;
};

#define yatc_interpreter_makeAnException(v1, v2) yatc_interpreter_makeAnException_(v1, v2, line, __FILE__, __LINE__)

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
      //dbgprintf("processed line: '%s' [%d], insideBrackets = %d, blocked = %d\n", line, i, insideBrackets, blocked);
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
	    else if (tp == YString || tp == YVector || tp == YSubroutine) {
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
  interp->context = calloc(_ctxsize, sizeof(YatcVariable*));
  interp->dlContext = yatc_dl_allocContext(9000);
  if (context)
    yatc_context_migrate(interp->context, context);
  return interp;
}

YatcInterpreterResult* yatc_interpreter_makeAnException_(unsigned line, const char* text, const char* ln, const char* fileCallable, unsigned lineCallable) {
  if (!text)
    return NULL;
  (void)(fileCallable);
  (void)(lineCallable);
  dbgprintf("Called yatc_interpreter_makeAnException from %s:%d for '%s' (%s)\n", fileCallable, lineCallable, ln, text);
  YatcInterpreterResult* res = malloc(sizeof(YatcInterpreterResult));
  res->success = 0;
  res->line = line + 1;
  res->description = calloc(strlen(text) + 1, sizeof(char));
  res->additionalData = NULL;
  if (!ln)
    res->lineReal = NULL;
  else {
    res->lineReal = calloc(strlen(ln) + 1, sizeof(char));
    strcpy(res->lineReal, ln);
  }
#ifndef __APPLE__
  void** backtraceTmpStorage = calloc(6, sizeof(void*));
  backtrace(backtraceTmpStorage, 6);
  res->backtrace = backtrace_symbols(backtraceTmpStorage, 6);
  free(backtraceTmpStorage);
#else
  res->backtrace = NULL;
#endif
  strcpy(res->description, text);
  return res;
}

char* yatc_cstring_clone(const char* str) {
  if (!str)
    return NULL;
  char* buf = calloc(strlen(str) + 1, sizeof(char));
  strcpy(buf, str);
  return buf;
}

char* yatc_interpreter_execInner(YatcInterpreter* interp, const char* line) {
  if (!line)
    return NULL;
  char* result = calloc(strlen(line) * 5, sizeof(char));
  if (yatc_cstring_howMany(line, '[') < 1) {
    free(result);
    interp->supportSubst = 0;
    dbgprintf("Executing trimmed '%s'\n", line);
    YatcInterpreterResult* rs = yatc_interpreter_exec(interp, line);
    if (!rs || !rs->additionalData) {
      dbgprintf("rs is NULL <%p> '%s'\n", rs, line);
      return NULL;
    } else if (strlen(line) >= 2 && line[0] == line[1] && line[0] == '!' && !rs->success) {
      char* lineButWritable = calloc(strlen(line) + 1, sizeof(char));
      strcpy(lineButWritable, line);
      return lineButWritable;
    }
    interp->supportSubst = 1;
    YatcVariable* vb = rs->additionalData; 
    dbgprintf("vb->mem = <%p>\nvb->type = %d\n", yatc_variable_get(vb), yatc_variable_get_type(vb));
    if (yatc_variable_get_type(vb) == YString || yatc_variable_get_type(vb) == YVector || yatc_variable_get_type(vb) == YSubroutine)
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
  unsigned insideCurlyBrackets = 0;
  for (unsigned i = 0; i < strlen(line); i++) {
    char character = line[i];
    if (character == '[' && insideCurlyBrackets < 1)
     insideBrackets += 1;
    else if (character == ']' && insideCurlyBrackets < 1) {
      insideBrackets -= 1;
      if (insideBrackets == 0) {
	dbgprintf("Calling execInner for '%s'\n", bufForLine);
	char* toAppend = yatc_interpreter_execInner(interp, bufForLine);
	dbgprintf("Call finished for '%s'\n", bufForLine);
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
    } else if (character == '{' || character == '}') {
      insideCurlyBrackets += 1;
      if (character == '}')
	insideCurlyBrackets -= 2;
      if (insideBrackets < 1)
	result[strlen(result)] = character;
      else
	bufForLine[strlen(bufForLine)] = character;
    } else if (insideBrackets >= 1)
      bufForLine[strlen(bufForLine)] = character;
    else
      result[strlen(result)] = character;
  }
  free(bufForLine);
  bufForLine = NULL;
  return result;
}

YatcInterpreterResult* yatc_interpreter_exec_(YatcInterpreter* interp, const char* line, const char* fnCallable, unsigned lineCallable) {
  (void)(fnCallable);
  (void)(lineCallable);
  dbgprintf("Called from %s:%d with line '%s'\n", fnCallable, lineCallable, line);
  if (!interp || !line) {
    //dbgprintf("Interpreter <%p> while parsing line '%s' <%p>\n", interp, line, line);
    return yatc_interpreter_makeAnException(0, NOTHING_WILL_BE_DONE_SORRY);
  }
  char** lineSplit = yatc_cstring_banalSplit(line, '\n');
  if (!lineSplit)
    return yatc_interpreter_makeAnException(0, NOTHING_WILL_BE_DONE_SORRY);
  //dbgprintf("Specified line:\n%s\n", line);
  YatcVariable* lastData = NULL;
  //yatc_csarray_dbgprintf(lineSplit);
  for (unsigned i = 0; i < yatc_csarray_length(lineSplit); i++) {
    char* lineOrig = lineSplit[i];
    // dbgprintf("lineOrig = '%s'\n", lineOrig);
    char* lineUnv = yatc_interpreter_unvars(lineOrig, interp->context, interp->scope);
    // dbgprintf("lineUnv = '%s'\n", lineUnv);
    char* lineTrim = yatc_cstring_trim(lineUnv);
    free(lineUnv);
    char* line = NULL;
    //dbgprintf("interp->supportSubst = %d\n", interp->supportSubst);
    if (interp->supportSubst && lineTrim && yatc_cstring_howMany(lineTrim, '[') >= 1) {
      line = yatc_interpreter_execInner(interp, lineTrim);
    } else if (lineTrim) {
      // dbgprintf("lineTrim = '%s'\n", lineTrim);
      line = calloc(strlen(lineTrim) + 1, sizeof(char));
      strcpy(line, lineTrim);
    }
    //dbgprintf("line = '%s'\n", line);
    free(lineTrim);
    if (line && strlen(line) >= 1 && line[0] != '#') {
      if (lastData) {
	free(lastData);
	lastData = NULL;
      }
      char** lineReallySplit = yatc_cstring_split(line, ' ');
      // yatc_csarray_dbgprintf(lineReallySplit);
      char* firstCommand = lineReallySplit[0];
      dbgprintf("firstCommand = '%s'\n", firstCommand);
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
      } else if (yatc_context_get_subr(interp->context, firstCommand)) {
	YatcVariable* implementation = yatc_context_get_subr(interp->context, firstCommand);
	char* code = yatc_variable_get(implementation);
	if (!code)
	  return yatc_interpreter_makeAnException(i, NOTHING_WILL_BE_DONE_SORRY);
	interp->supportSubst = 1;
	interp->scope += 1;
	char* argsBuf = calloc(yatc_csarray_length(lineReallySplit) * strlen(line), sizeof(char));
	strcpy(argsBuf, "/&&=");
	YatcVariable* vblV = yatc_variable_create("args", YVector, argsBuf, interp->scope);
	int* successfulMem = malloc(sizeof(int));
	*successfulMem = 1;
	YatcVariable* vblR = yatc_variable_create("_", YInteger, successfulMem, interp->scope);
	if (yatc_context_has(interp->context, "args", interp->scope)) {
	  yatc_context_unregister(interp->context, "args", interp->scope, vblV);
	  yatc_context_unregister(interp->context, "_", interp->scope, vblR);
	} else {
	  yatc_context_register(interp->context, vblV);
	  yatc_context_register(interp->context, vblR);
	}
	for (unsigned i = 1; i < yatc_csarray_length(lineReallySplit); i++) {
	  strcat(argsBuf, lineReallySplit[i]);
	  strcat(argsBuf, "\r");
	  char* stringifiedNum = calloc(4, sizeof(char));
	  sprintf(stringifiedNum, "%d", i);
	  YatcVariable* vbl = yatc_variable_create(stringifiedNum, YString, lineReallySplit[i], interp->scope);
	  if (yatc_context_has(interp->context, stringifiedNum, interp->scope))
	    yatc_context_unregister(interp->context, stringifiedNum, interp->scope, vbl);
	  else
	    yatc_context_register(interp->context, vbl);
	}
	// dbgprintf("About to run:\n%s\n", code);
	YatcInterpreterResult* rslt = yatc_interpreter_exec(interp, code);
	interp->scope -= 1;
	if (!rslt)
	  return yatc_interpreter_makeAnException(i, NOTHING_WILL_BE_DONE_SORRY);
	else if (!rslt->success) {
	  rslt->line = i;
	  return rslt;
	} else {
	  lastData = yatc_context_get(interp->context, "_", interp->scope + 1);
	  rslt->additionalData = NULL;
	  free(rslt);
	}
      } else if (strcmp(firstCommand, "cond") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Please specify the condition that you want to evaluate.");
	char* condition = yatc_interpreter_unvars(lineReallySplit[1], interp->context, interp->scope);
	unsigned result = yatc_expressions_conditionMet(condition);
	int* resultCast = malloc(sizeof(int));
	*(resultCast) = (int)(result);
	lastData = yatc_variable_create("", YInteger, resultCast, interp->scope);
      } else if (strcmp(firstCommand, "return") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Return what?");
	YatcVariable* amazingResult = yatc_variable_create("_", YString, yatc_cstring_clone(lineReallySplit[1]), interp->scope);
	if (yatc_context_has(interp->context, "_", interp->scope))
	  yatc_context_unregister(interp->context, "_", interp->scope, amazingResult);
	else
	  yatc_context_register(interp->context, amazingResult);
	lastData = yatc_variable_create("", YString, yatc_cstring_clone(lineReallySplit[1]), interp->scope);
      } else if (strcmp(firstCommand, "info") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Please specify the type of information that you would like to retreive.");
	char* what = lineReallySplit[1];
	char* info = calloc(256, sizeof(char));
	if (strcmp(what, "version") == 0 || strcmp(what, "patchlevel") == 0)
	  strcpy(info, YATC_VERSION);
	else if (strcmp(what, "os") == 0)
	  strcpy(info, yatc_lowlevel_get_os());
	else if (strcmp(what, "pwd") == 0)
	  getcwd(info, 256);
	else if (strcmp(what, "time") == 0) {
	  time_t thisTime = time(NULL);
	  char* stringifiedTime = ctime(&thisTime);
	  stringifiedTime[strlen(stringifiedTime) - 1] = '\0';
	  strcpy(info, stringifiedTime);
	  free(stringifiedTime);
	} else if (strcmp(what, "random") == 0) {
	  int value = rand();
	  sprintf(info, "%d", value);
	} else if (strcmp(what, "mode") == 0)
	  strcpy(info, yatc_io_streamsImplementation());
	else
	  return yatc_interpreter_makeAnException(i, "Unknown parameter specified to info.");
	lastData = yatc_variable_create("", YString, info, interp->scope);
      } else if (strcmp(firstCommand, "source") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Please specify at least one file with valid Yatc code.");
	for (unsigned i = 1; i < yatc_csarray_length(lineReallySplit); i++) {
	  char* fn = lineReallySplit[i];
	  if (!yatc_io_fileExists(fn))
	    return yatc_interpreter_makeAnException(i, "No such file or directory.");
	  char* code = yatc_io_readAll(fn);
	  if (!code)
	    return yatc_interpreter_makeAnException(i, NOTHING_WILL_BE_DONE_SORRY);
	  YatcInterpreterResult* rslt = yatc_interpreter_exec(interp, code);
	  if (!rslt)
	    return yatc_interpreter_makeAnException(i, NOTHING_WILL_BE_DONE_SORRY);
	  else if (!rslt->success) {
	    rslt->line = i;
	    return rslt;
	  } else
	    free(rslt);
	  free(code);
	}
      } else if (strcmp(firstCommand, "dl") == 0) {
	yatc_csarray_dbgprintf(lineReallySplit);
	if (yatc_csarray_length(lineReallySplit) < 3)
	  return yatc_interpreter_makeAnException(i, "Please specify the name of the dynamic library and at least one valid subroutine name.");
	char* fn = lineReallySplit[1];
	if (!yatc_io_fileExists(fn))
	  return yatc_interpreter_makeAnException(i, "The specified dynamic library does not exist.");
	if (!interp->dlContext)
	  return yatc_interpreter_makeAnException(i, "This build of Yatc was compiled without dynamic library loading support.");
	void* libHandle = yatc_dl_dlopen(fn, RTLD_LAZY);
	if (!libHandle)
	  return yatc_interpreter_makeAnException(i, "dlopen(fn, RTLD_LAZY) had just returned NULL.");
	for (unsigned i = 2; i < yatc_csarray_length(lineReallySplit); i++) {
	  char* symname = lineReallySplit[i];
	  if (!yatc_dl_import(libHandle, symname, interp->dlContext))
	    return yatc_interpreter_makeAnException(i, "Symbol importing failed.");
	}
      } else if (strcmp(firstCommand, "tolower") == 0 || strcmp(firstCommand, "toupper") == 0 || strcmp(firstCommand, "reverse") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Please specify the string that you want to transform.");;
	unsigned flag = YATC_STRING_LOWERCASE;
	if (strcmp(firstCommand, "toupper") == 0)
	  flag = YATC_STRING_UPPERCASE;
	else if (strcmp(firstCommand, "reverse") == 0)
	  flag = YATC_STRING_REVERSE;
	char* resultingBuf = yatc_cstring_transformCase(lineReallySplit[1], flag);
	lastData = yatc_variable_create("", YString, resultingBuf, interp->scope);
      } else if (yatc_dl_contextGet(interp->dlContext, firstCommand)) {
	YatcImportedCSymbol* symbl = yatc_dl_contextGet(interp->dlContext, firstCommand);
	YatcVariable** vrbls = calloc(yatc_csarray_length(lineReallySplit) + 1, sizeof(YatcVariable*));
	for (unsigned i = 1; i < yatc_csarray_length(lineReallySplit); i++) {
	  YatcInterpreterResult* rslt = yatc_interpreter_exec(interp, lineReallySplit[i]);
	  if (!rslt || !rslt->success || !rslt->additionalData) {
	    free(rslt);
	    continue;
	  }
	  yatc_variable_set_scope(rslt->additionalData, 255);
	  char* argStr = calloc(yatc_csarray_length(lineReallySplit), sizeof(char));
	  sprintf(argStr, "%d", i);
	  yatc_variable_set_name(rslt->additionalData, argStr);
	  free(argStr);
	  yatc_context_register(vrbls, rslt->additionalData);
	  rslt->additionalData = NULL;
	  free(rslt);
	}
	lastData = symbl->mem(vrbls);
	if (lastData)
	  yatc_variable_set_scope(lastData, interp->scope);
	yatc_context_goodbye(vrbls);
      } else if (strcmp(firstCommand, "sub") == 0) {
	//yatc_csarray_dbgprintf(lineReallySplit);
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Please specify the name and the algorithm of the subroutine that you're about to implement.");
	char* name = lineReallySplit[1];
	char* code = lineReallySplit[2];
	if (yatc_context_get_subr(interp->context, line))
	  return yatc_interpreter_makeAnException(i, "Subroutine with the same name already exists.");
	char* codeCloned = calloc(strlen(code) + 1, sizeof(char));
	strcpy(codeCloned, code);
	YatcVariable* vb = yatc_variable_create(name, YSubroutine, codeCloned, 0);
	yatc_context_register(interp->context, vb);
      } else if (strcmp(firstCommand, "set") == 0 || strcmp(firstCommand, "const") == 0) {
	//dbgprintf("yatc_csarray_length(lineReallySplit) = %d\n", yatc_csarray_length(lineReallySplit));
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
	  //yatc_context_dbgprintf(interp->context);
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
      } else if (strcmp(firstCommand, "vector") == 0) {
	char* vectorV = calloc(_vssize * _vssize, sizeof(char));
	strcpy(vectorV, "/&&=");
	for (unsigned i = 1; i < yatc_csarray_length(lineReallySplit); i++) {
	   strcat(vectorV, lineReallySplit[i]);
	   strcat(vectorV, "\r");
	}
	lastData = yatc_variable_create("", YVector, vectorV, interp->scope);
      } else if (strcmp(firstCommand, "length") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Please specify the variable the length of which shall be counted.");
	char* arg = lineReallySplit[1];
	void* mem = malloc(sizeof(int));
	if (yatc_vector_indeed(arg))
	  *(int*)(mem) = yatc_vector_length(arg);
	else
	  *(int*)(mem) = strlen(arg);
	lastData = yatc_variable_create("", YInteger, mem, interp->scope);
      } else if (strcmp(firstCommand, "foreach") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 4)
	  return yatc_interpreter_makeAnException(i, "Please specify the name of the iterator, the iterable and the code to execute.");
	//yatc_csarray_dbgprintf(lineReallySplit);
	char* arg = lineReallySplit[2];
	char* vn = lineReallySplit[1];
	YatcVariable* vrbl = yatc_variable_create(vn, YString, "", interp->scope);
	if (yatc_context_has(interp->context, vn, interp->scope))
	  yatc_context_unregister(interp->context, vn, interp->scope, vrbl);
	else
	  yatc_context_register(interp->context, vrbl);
	// interp->scope += 1;
	if (yatc_vector_indeed(arg)) {
	  char** splitCNative = yatc_vector_convert(arg);
	  for (unsigned b = 0; b < yatc_csarray_length(splitCNative); b++) {
	    char* cnativeIterable = calloc(strlen(splitCNative[b]) + 1, sizeof(char));
	    strcpy(cnativeIterable, splitCNative[b]);
	    if (atoi(cnativeIterable) != 0 || strcmp(cnativeIterable, "0") == 0) {
	      int* tmpMem = malloc(sizeof(int));
	      *tmpMem = atoi(cnativeIterable);
	      yatc_context_unregister(interp->context, vn, interp->scope, yatc_variable_create(vn, YInteger, tmpMem, interp->scope));
	    } else
	      yatc_context_unregister(interp->context, vn, interp->scope, yatc_variable_create(vn, YString, cnativeIterable, interp->scope));
	    YatcInterpreterResult* rslt = yatc_interpreter_exec(interp, lineReallySplit[3]);
	    if (!rslt->success) {
	      rslt->line = i;
	      return rslt;
	    }
	    free(rslt);
	    free(cnativeIterable);
	  }
	  free(splitCNative);
	} else {
	  for (unsigned b = 0; b < strlen(arg); b++) {
	    char* bufTmp = calloc(2, sizeof(char));
	    bufTmp[0] = arg[b];
	    bufTmp[1] = '\0';
	    yatc_variable_set(vrbl, bufTmp);
	    YatcInterpreterResult* rslt = yatc_interpreter_exec(interp, lineReallySplit[3]);
	    if (!rslt->success) {
	      rslt->line = i;
	      return rslt;
	    }
	    free(rslt);
	    free(bufTmp);
	  }
	}
      } else if (strcmp(firstCommand, "vappend") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 3)
	  return yatc_interpreter_makeAnException(i, "Please specify the name of the variable pointing to the vector as well as at least one item to append.");
	char* vname = lineReallySplit[1];
	if (!yatc_context_has(interp->context, vname, interp->scope))
	  return yatc_interpreter_makeAnException(i, "The specified variable does not exist or does not point to a valid vector.");
	YatcVariable* vbl = yatc_context_get(interp->context, vname, interp->scope);
	char* mem = yatc_variable_get(vbl);
	if (!mem || yatc_variable_get_type(vbl) != YVector || !yatc_vector_indeed(mem))
	  return yatc_interpreter_makeAnException(i, "The specified variable does not exist or does not point to a valid vector.");
	for (unsigned b = 2; b < yatc_csarray_length(lineReallySplit); b++) {
	  if (yatc_vector_indeed(lineReallySplit[b]))
	    return yatc_interpreter_makeAnException(i, "Yatc vectors cannot contain copies of itself or any other vector instances.");
	  strcat(mem, lineReallySplit[b]);
	  strcat(mem, "\r");
	}
      } else if (strcmp(firstCommand, "index") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 3)
	  return yatc_interpreter_makeAnException(i, "Please specify the variable as well as the position of data that you want to retreive.");
	char* varv = lineReallySplit[1];
	char* whatOrig = lineReallySplit[2];
	unsigned what = atoi(whatOrig);
	if (yatc_vector_indeed(varv)) {
	  if (what >= yatc_vector_length(varv))
	    return yatc_interpreter_makeAnException(i, "Index out of vector bounds.");
	  char** vectorConv = yatc_vector_convert(varv);
	  char* mem = calloc(strlen(vectorConv[what]) + 1, sizeof(char));
	  strcpy(mem, vectorConv[what]);
	  lastData = yatc_variable_create("", YString, mem, interp->scope);
	} else {
	  if (what >= strlen(varv))
	    return yatc_interpreter_makeAnException(i, "Index out of pointer bounds.");
	  char* mem = calloc(2, sizeof(char));
	  mem[0] = varv[what];
	  mem[1] = '\0';
	  lastData = yatc_variable_create("", YString, mem, interp->scope);
	}
      } else if (strcmp(firstCommand, "if") == 0) {
	//yatc_csarray_dbgprintf(lineReallySplit);
	if (yatc_csarray_length(lineReallySplit) < 3 || ((yatc_csarray_length(lineReallySplit) % 3) != 0))
	  return yatc_interpreter_makeAnException(i, "Please specify at least one condition and the subsequent algorithm to run.");
	unsigned atLeastOneDid = 0;
	dbgprintf("reached if, length = %d\n", yatc_csarray_length(lineReallySplit));
	for (unsigned b = 0; b < yatc_csarray_length(lineReallySplit); b += 3) {
	  dbgprintf("b = %d\n", b);
	  char* keyword = lineReallySplit[b];
	  char* condition = yatc_interpreter_unvars(lineReallySplit[b + 1], interp->context, interp->scope);
	  char* conditionOrig = condition;
	  condition = yatc_interpreter_execInner(interp, condition);
	  interp->supportSubst = 1;
	  free(conditionOrig);
	  char* code = lineReallySplit[b + 2];
	  dbgprintf("keyword = '%s', condition = '%s', code:\n%s\n", keyword, condition, code);
	  if (atLeastOneDid)
	    break;
	  if (strcmp(keyword, "if") == 0 || strcmp(keyword, "elseif") == 0) {
	    atLeastOneDid = yatc_expressions_conditionMet(condition);
	    dbgprintf("condition = '%s', met = %d\n", condition, atLeastOneDid);
	    if (atLeastOneDid) {
	      YatcInterpreterResult* rs = yatc_interpreter_exec(interp, code);
	      if (!rs)
		return yatc_interpreter_makeAnException(i, NOTHING_WILL_BE_DONE_SORRY);
	      else if (rs->success)
		free(rs);
	      else
		return rs;
	    }
	  } else if (strcmp(keyword, "else") == 0) {
	    atLeastOneDid = 1;
	    YatcInterpreterResult* rs = yatc_interpreter_exec(interp, code);
	    if (!rs)
	      return yatc_interpreter_makeAnException(i, NOTHING_WILL_BE_DONE_SORRY);
	    else if (rs->success)
	      free(rs);
	    else
	      return rs;
	  } else
	    return yatc_interpreter_makeAnException(i, "Unknown conditional keyword (must be if, elseif or else)");
	}
      } else if (strcmp(firstCommand, "!!") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Please specify the description of the exception.");
	return yatc_interpreter_makeAnException(i, lineReallySplit[1]);
      } else if (strcmp(firstCommand, "while") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 3)
	  return yatc_interpreter_makeAnException(i, "Please specify the condition and the algorithm.");
	char* condition = lineReallySplit[1];
	dbgprintf("Original condition: '%s'\n", condition);
	char* code = lineReallySplit[2];
	unsigned conditionValid = 1;
	while (conditionValid == 1) {
	  char* substCondition = yatc_interpreter_unvars(condition, interp->context, interp->scope);
	  dbgprintf("Condition after yatc_interpreter_unvars treatment: %s (before was '%s')\n", substCondition, condition);
	  conditionValid = yatc_expressions_conditionMet(substCondition);
	  free(substCondition);
	  if (!conditionValid)
	    break;
	  YatcInterpreterResult* rslt = yatc_interpreter_exec(interp, code);
	  if (!rslt)
	    return yatc_interpreter_makeAnException(i, NOTHING_WILL_BE_DONE_SORRY);
	  else if (!rslt->success)
	    return rslt;
	  else
	    free(rslt);
	}
      } else if (strcmp(firstCommand, "incr") == 0 || strcmp(firstCommand, "decr") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Please specify at least the name of the variable that you want to increment.");
	int coef = 1;
	for (unsigned b = 1; b < yatc_csarray_length(lineReallySplit); b++) {
	  char* vn = lineReallySplit[b];
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
      } else if (strcmp(firstCommand, "exists") == 0) {
	if (yatc_csarray_length(lineReallySplit) < 2)
	  return yatc_interpreter_makeAnException(i, "Please specify the filename first.");
	int* result = malloc(sizeof(int));
	unsigned resultReal = yatc_io_fileExists(lineReallySplit[1]);
	*(result) = resultReal;
	lastData = yatc_variable_create("", YInteger, result, interp->scope);
      } else {
	dbgprintf("Type casting for '%s'\n", line);
	int numericValue = atoi(line);
	YatcCommonType tp = YString;
	void* value = calloc(strlen(line) + 1, sizeof(char));
	strcpy(value, line);
	if (atoi(line) != 0 || strcmp(line, "0") == 0) {
	  tp = YInteger;
	  value = malloc(sizeof(int));
	  *(int*)(value) = numericValue;
	} else if (yatc_vector_indeed(line))
	  tp = YVector;
	lastData = yatc_variable_create("", tp, value, interp->scope);
	dbgprintf("lastData = <%p>\n", lastData);
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
  dbgprintf("At the end, lastData = <%p>, line = '%s'\n", lastData, line);
  result->additionalData = lastData;
  result->lineReal = NULL;
  result->backtrace = NULL;
  return result;
}

unsigned yatc_vector_indeed(const char* vc) {
  if (!vc)
    return 0;
  return (strlen(vc) >= 4 && vc[0] == '/' && vc[1] == vc[2] && vc[1] == '&' && vc[3] == '=');
}

unsigned yatc_vector_length(const char* vc) {
  if (!yatc_vector_indeed(vc))
    return 0;
  return yatc_cstring_howMany(vc, '\r');
}


void yatc_interpreter_goodbye(YatcInterpreter* interp) {
  if (!interp)
    return;
  yatc_context_goodbye(interp->context);
  yatc_dl_contextGoodbye(interp->dlContext);
  free(interp);
}

char** yatc_vector_convert(const char* vc) {
  if (!yatc_vector_indeed(vc))
    return NULL;
  char** array = yatc_cstring_banalSplit(vc, '\r');
  array[0] = (array[0] + 4);
  return array;
}
