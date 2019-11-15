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
char** yatc_cstring_banalSplit(const char* orig, const char token);
unsigned yatc_cstring_howMany(const char* orig, const char token);
char* yatc_cstring_transformCase(const char* orig, unsigned flag);
char** yatc_csarray_removeEmpty(char** orig);
unsigned yatc_csarray_length(char** orig);
void yatc_csarray_fprintf(FILE* where, char** array);
char* yatc_cstring_trim(const char* orig);
char* yatc_cstring_substring(const char* orig, unsigned start, unsigned end);
unsigned yatc_csarray_has(char** array, const char* what);
char** yatc_csarray_part(char** array, unsigned start, unsigned end);
const char* yatc_lowlevel_get_os();

#ifdef YATC_DEBUG
#define dbgprintf(what, ...) fprintf(stderr, what, __VA_ARGS__)
#define yatc_csarray_dbgprintf(array) yatc_csarray_fprintf(stdout, array)
#else
#define dbgprintf(what, ...) (void)(what)
#define yatc_csarray_dbgprintf(array) (void)(array)
#endif

#define yatc_csarray_printf(array) yatc_csarray_fprintf(stdout, array)
#define yatc_array_append(thearray, itslength, what, reallocation) { \
					      thearray[itslength] = what; \
					      itslength += 1; \
					      what = reallocation; }


#endif
