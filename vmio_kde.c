#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vmio.h"

#define _bfm 5056

#ifdef _WIN32
#error "This will only work on Linux/FreeBSD with KDE 4 or 5."
#endif

void yatc_io_simplestOutput(const char* text) {
  if (!text) {
    yatc_io_simplestOutput("YSomething");
    return;
  }
  char* cmd = calloc(strlen(text) + 256, sizeof(char));
  strcpy(cmd, "kdialog --title 'Current application says' --msgbox '");
  strcat(cmd, text);
  strcat(cmd, "'");
  //printf("%s\n", cmd);
  system(cmd);
  free(cmd);
}

char* yatc_io_prompt(const char* what) {
  if (!what)
    return yatc_io_prompt("Please enter a string or a numerical value");
  char* cmd = calloc(strlen(what) + 256, sizeof(char));
  strcpy(cmd, "kdialog --title 'Current application asks' --inputbox '");
  strcat(cmd, what);
  strcat(cmd, "' '' > /tmp/yatc_kde.sock 2>/dev/null");
  system(cmd);
  char* result = yatc_io_readAll("/tmp/yatc_kde.sock");
  yatc_io_unlink("/tmp/yatc_kde.sock");
  free(cmd);
  return result;
}