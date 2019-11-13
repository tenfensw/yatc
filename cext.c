#include "cext.h"

char* yatc_cstring_trim(const char* orig) {
  if (!orig || strlen(orig) < 1)
    return NULL;
  char* new = calloc(strlen(orig) + 1, sizeof(char));
  unsigned offset = 0;
  if (orig[0] == '\t' || orig[0] == ' ') {
    while (offset < strlen(orig) && (orig[offset] == ' ' || orig[offset] == '\t'))
      offset += 1;
  }
  unsigned index = 0;
  for (unsigned i = offset; i < strlen(orig); i++) {
    new[index] = orig[i];
    index += 1;
  }
  return new;
}  

char** yatc_cstring_banalSplit(const char* orig, const char token) {
  if (!orig || strlen(orig) < 1)
    return NULL;
  char** allocatedBuffer = calloc(yatc_cstring_howMany(orig, token) + 2, sizeof(char*));
  unsigned bufId = 0;
  unsigned insideBrackets = 0;
  for (unsigned i = 0; i < strlen(orig); i++) {
    if (!(allocatedBuffer[bufId]))
      allocatedBuffer[bufId] = calloc(strlen(orig), sizeof(char));
    char* where = allocatedBuffer[bufId];
    if (orig[i] == token && insideBrackets < 1)
      bufId += 1;
    else if (orig[i] == '{' || orig[i] == '}') {
      insideBrackets += 1;
      if (orig[i] == '}')
	insideBrackets -= 2;
      where[strlen(where)] = orig[i];
    } else
      where[strlen(where)] = orig[i];
  }
  return allocatedBuffer;
}

char** yatc_cstring_split(const char* orig, const char token) {
  if (!orig || token == '\0')
    return NULL;
  unsigned allocBufCount = yatc_cstring_howMany(orig, token) * 4;
  if (allocBufCount < 1) {
    char** fakedBuffer = calloc(2, sizeof(char*));
    fakedBuffer[0] = calloc(strlen(orig) + 1, sizeof(char));
    strcpy(fakedBuffer[0], orig);
    return fakedBuffer;
  }
  char** resultingBuffer = calloc(allocBufCount + 1, sizeof(char*));
  unsigned index = 0;
  unsigned insideQuotes = 0;
  int insideBrackets = 0;
  unsigned blocked = 0;
  for (unsigned i = 0; i < strlen(orig); i++) {
    char* where = NULL;
    if (!(resultingBuffer[index]))
      resultingBuffer[index] = calloc(YATC_STRING_MAXLEN, sizeof(char));
    where = resultingBuffer[index];
    char current = orig[i];
    if ((current == '\'' || current == '"') && !blocked && insideBrackets < 1) {
      insideQuotes = !(insideQuotes);
      index += 1;
    } else if ((current == '{' || current == '}') && !blocked) {
      insideBrackets += 1;
      if (current == '}')
	insideBrackets -= 2;
      if (insideBrackets == 1 || insideBrackets == 0)
	index += 1;
    } else if ((current == '[' || current == ']') && !blocked) {
      insideQuotes = (current == '[');
      where[strlen(where)] = current;
    } else if (current == '\\') {
      blocked = !(blocked);
      if (!blocked)
	where[strlen(where)] = current;
      continue;
    } else if (current == token && insideBrackets < 1 && !blocked && !insideQuotes)
      index += 1;
    else
      where[strlen(where)] = current;
    if (blocked)
      blocked = 0;
  }
  char** result = yatc_csarray_removeEmpty(resultingBuffer);
  free(resultingBuffer);
  return result;
}

unsigned yatc_csarray_length(char** orig) {
   unsigned count = 0;
   while (1) {
     if (orig[count])
       count += 1;
     else
       break;
   }
   return count;
}

char** yatc_csarray_removeEmpty(char** orig) {
  if (!orig)
    return NULL;
  unsigned origLen = yatc_csarray_length(orig);
  if (origLen < 1)
    return orig;
  char** result = calloc(origLen + 1, sizeof(char*));
  unsigned count = 0;
  for (unsigned i = 0; i < origLen; i++) {
    char* str = orig[i];
    if (!str)
      break;
    else if (strlen(str) >= 1) {
      result[count] = calloc(strlen(str) + 1, sizeof(char));
      strcpy(result[count], orig[i]);
      count += 1;
    }
  }
  return result;
}

unsigned yatc_cstring_howMany(const char* orig, const char token) {
  if (!orig || token == '\0')
    return 0;
  unsigned count = 0;
  for (unsigned i = 0; i < strlen(orig); i++) {
    if (orig[i] == token)
      count += 1;
  }
  return count;
}

void yatc_csarray_fprintf(FILE* where, char** array) {
  unsigned index = 0;
  while (1) {
    if (array[index])
      fprintf(where, "[%d] %s\n", index, array[index]);
    else
      break;
    index += 1;
  }
}

char* yatc_cstring_transformCase(const char* orig, unsigned flag) {
  if (!orig || (flag != YATC_STRING_LOWERCASE && flag != YATC_STRING_UPPERCASE && flag != YATC_STRING_REVERSE))
    return NULL;
  char* str = calloc(strlen(orig) + 1, sizeof(char));
  if (flag == YATC_STRING_REVERSE) {
    unsigned index = 0;
    for (int i = strlen(orig) - 1; i >= 0; i--) {
      str[index] = orig[i];
      index += 1;
    }
  } else if (flag == YATC_STRING_LOWERCASE || flag == YATC_STRING_UPPERCASE) {
    for (unsigned i = 0; i < strlen(orig); i++) {
      char resultingChar = tolower(orig[i]);
      if (flag == YATC_STRING_UPPERCASE)
	resultingChar = toupper(resultingChar);
      str[i] = resultingChar;
    }
  }
  return str;
}
