#include "cext.h"
#include <stdio.h>

int main(int argc, const char** argv) {
  char* orig = calloc(256, sizeof(char));
  strcpy(orig, "Meowie meowie meowie meow! ;-)");
  if (argc > 1)
    strcpy(orig, argv[1]);
  printf("Original: %s\n", orig);
  char* lw = yatc_cstring_transformCase(orig, YATC_STRING_LOWERCASE);
  char* up = yatc_cstring_transformCase(orig, YATC_STRING_UPPERCASE);
  char* rv = yatc_cstring_transformCase(orig, YATC_STRING_REVERSE);
  printf("Lowercase: %s\nUppercase: %s\nReverse: %s\n", lw, up, rv);
  free(lw);
  free(up);
  free(rv);
  free(orig);
  return 0;
}