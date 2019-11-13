#include "vmio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

unsigned yatc_io_unlink(const char* fn) {
  if (!fn)
    return 0;
#ifdef __linux__
  return (unlink(fn) == 0);
#else
  return (remove(fn) == 0);
#endif
}

