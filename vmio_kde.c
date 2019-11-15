//
// Yatc - Yet Another Tcl Clone
// An embeddable modern scripting language with Tcl-like syntax
// Copyright (C) Tim K/RoverAMD 2019 <timprogrammer@rambler.ru>.
// 
// vmio_kde.c - Implementation of several IO functions via KDE's
// kdialog
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vmio.h"

#define _bfm 5056

#ifdef _WIN32
#error "This will only work on Linux/FreeBSD with KDE 4 or 5."
#endif

void yatc_io_kdialogOutput(const char* text, const char* dlgPreferrence) {
  if (!dlgPreferrence) {
    yatc_io_kdialogOutput(text, "msgbox");
    return;
  }
  if (!text) {
    yatc_io_kdialogOutput("YSomething", dlgPreferrence);
    return;
  }
  char* cmd = calloc(strlen(text) + 256, sizeof(char));
  strcpy(cmd, "kdialog --title 'Current application says' --");
  strcat(cmd, dlgPreferrence);
  strcat(cmd, " '");
  strcat(cmd, text);
  strcat(cmd, "'");
  //printf("%s\n", cmd);
  system(cmd);
  free(cmd);
}

const char* yatc_io_streamsImplementation() {
  return "kde4";
}

void yatc_io_simplestOutput(const char* text) {
  yatc_io_kdialogOutput(text, "msgbox");
}

void yatc_io_errorOutput(const char* text) {
  yatc_io_kdialogOutput(text, "error");
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