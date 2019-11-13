#include "vmio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void yatc_io_simplestOutput(const char* text) {
  if (!text)
    printf("VSomething\n");
  else
    printf("%s\n", text);
}

unsigned yatc_io_fileExists(const char* fn) {
  if (!fn || access(fn, F_OK) == -1)
    return 0;
  return 1;
}

char* yatc_io_readAll(const char* fn) {
  if (!fn)
    return NULL;
  FILE* fdesc = fopen(fn, "r");
  if (!fdesc)
    return NULL;
  char mch = '\0';
  char* buf = calloc(300000, sizeof(char));
  unsigned index = 0;
  while ((mch = fgetc(fdesc)) != EOF) {
    buf[index] = mch;
    index += 1;
  }
  fclose(fdesc);
  return buf;
}
