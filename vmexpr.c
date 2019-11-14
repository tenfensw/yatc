#include <string.h>
#include "vmexpr.h"

int yatc_expressions_feedMath(const char* stringified, unsigned* ok) {
  unsigned whatToSetToOK = 0;
  if (!stringified) {
    if (ok) 
      *(ok) = 0;
    return -1;
  }
  int errst = 0;
  double resultOrig = te_interp(stringified, &errst);
  int result = -1;
  if (resultOrig != NAN) {
    whatToSetToOK = 1;
    result = (int)(resultOrig);
  }
  if (ok)
    *(ok) = whatToSetToOK;
  return result;
}

unsigned** yatc_expressions_measureByOperators(char** input) {
  if (!input)
    return NULL;
  unsigned start = 0;
  unsigned end = yatc_csarray_length(input);
  unsigned** maxExprArray = calloc(256, sizeof(unsigned*));
  unsigned maxIndex = 0;
  for (unsigned i = 0; i < yatc_csarray_length(input); i++) {
    if (strcmp(input[i], "&&") == 0 || strcmp(input[i], "||") == 0) {
      end = i - 1;
      maxExprArray[maxIndex] = calloc(3, sizeof(unsigned));
      maxExprArray[maxIndex][0] = start;
      maxExprArray[maxIndex][1] = end;
      maxExprArray[maxIndex][2] = strcmp(input[i], "||");
      start = i + 1;
      end = yatc_csarray_length(input);
    }
  }
  return maxExprArray;
}

unsigned yatc_expressions_conditionMet(const char* cond) {
  if (!cond)
    return 0;
  char** splitCond = yatc_cstring_split(cond, ' ');
  unsigned invert = 0;
  if (strcmp(splitCond[0], "!") == 0) {
    invert = 1;
    splitCond = splitCond + 1;
  }
  unsigned** indices = yatc_expressions_measureByOperators(splitCond);
  unsigned index = 0;
  unsigned condvg = 1;
  unsigned prevop = 0;
  while (indices[index] != NULL) {
    unsigned condv = 0;
    unsigned* slice = indices[index];
    char** subsequentPart = yatc_csarray_part(splitCond, slice[0], slice[1]);
    char* centralKeyword = NULL;
    char* arg1 = NULL;
    char* arg2 = NULL;
    if (yatc_csarray_length(subsequentPart) == 3) {
      centralKeyword = subsequentPart[1];
      arg1 = subsequentPart[0];
      arg2 = subsequentPart[1];
      if (strcmp(centralKeyword, "==") == 0)
	condv = (strcmp(arg1, arg2) == 0);
      else if (strcmp(centralKeyword, "!=") == 0)
	condv = (strcmp(arg1, arg2) != 0);
      else if (strlen(centralKeyword) >= 1 && (centralKeyword[0] == '<' || centralKeyword[0] == '>')) {
	int firstNum = atoi(arg1);
	int secondNum = atoi(arg2);
	if (centralKeyword[0] == '<')
	  condv = (firstNum < secondNum);
	else
	  condv = (firstNum > secondNum);
	if (strlen(centralKeyword) >= 2 && centralKeyword[1] == '=')
	  condv = (condv || firstNum == secondNum);
      }
    } else if (yatc_csarray_length(subsequentPart) == 1) {
      arg1 = subsequentPart[0];
      if (arg1 && strcmp(arg1, "0") != 0)
	condv = 1;
    }
    index += 1;
    prevop = slice[2];
    if (prevop)
      condvg = (condvg || condv);
    else
      condvg = (condvg && condv);
  }
  free(indices);
  free(splitCond);
  if (invert)
    return !condvg;
  return condvg;
}