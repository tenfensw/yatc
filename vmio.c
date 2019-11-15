//
// Yatc - Yet Another Tcl Clone
// An embeddable modern scripting language with Tcl-like syntax
// Copyright (C) Tim K/RoverAMD 2019 <timprogrammer@rambler.ru>.
// 
// vmio.c - Implementation of file IO functions
//

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

unsigned yatc_io_fileOutput(const char* fn, const char* what) {
  if (!fn || !what)
    return 0;
  FILE* desc = fopen(fn, "w");
  if (!desc)
    return 0;
  fputs(what, desc);
  fclose(desc);
  return 1;
}

