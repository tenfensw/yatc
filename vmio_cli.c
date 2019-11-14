#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "vmio.h"

#define _bfm 5056

void yatc_io_simplestOutput(const char* text) {
  if (!text)
    printf("VSomething\n");
  else
    printf("%s\n", text);
}

const char* yatc_io_streamsImplementation() {
  return "cli";
}

void yatc_io_errorOutput(const char* text) {
  if (!text)
    fprintf(stderr, "VSomething\n");
  else
    fprintf(stderr, "%s\n", text);
}

char* yatc_io_prompt(const char* what) {
  if (what)
    printf("%s ", what);
  char* buf = calloc(_bfm, sizeof(char));
  char myc = 'm';
  unsigned index = 0;
  while (1) {
    myc = getchar();
    if (myc == '\0' || myc == '\n' || index >= _bfm)
      break;
    else {
      buf[index] = myc;
      index += 1;
    }
  }
  return buf;
}