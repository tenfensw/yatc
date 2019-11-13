#ifndef YATC_CEXT
#define YATC_CEXT

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define YATC_STRING_LOWERCASE 0
#define YATC_STRING_UPPERCASE 1
#define YATC_STRING_REVERSE 2
#define YATC_STRING_MAXLEN (1024 * 1024)

char** yatc_cstring_split(const char* orig, const char token);
unsigned yatc_cstring_howMany(const char* orig, const char token);
char* yatc_cstring_transformCase(const char* orig, unsigned flag);
char** yatc_csarray_removeEmpty(char** orig);
unsigned yatc_csarray_length(char** orig);
void yatc_csarray_fprintf(FILE* where, char** array);
char* yatc_cstring_trim(const char* orig);

#define yatc_csarray_printf(array) yatc_csarray_fprintf(stdout, array)
#define yatc_array_append(thearray, itslength, what, reallocation) { \
					      thearray[itslength] = what; \
					      itslength += 1; \
					      what = reallocation; }

#endif