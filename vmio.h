//
// Yatc - Yet Another Tcl Clone
// An embeddable modern scripting language with Tcl-like syntax
// Copyright (C) Tim K/RoverAMD 2019 <timprogrammer@rambler.ru>.
// 
// vmio.h - Prototypes of input-output-related functions
//

#ifndef YATC_VMIO
#define YATC_VMIO

void yatc_io_simplestOutput(const char* text);
void yatc_io_errorOutput(const char* text);
const char* yatc_io_streamsImplementation();
char* yatc_io_prompt(const char* what);
unsigned yatc_io_unlink(const char* fn);
unsigned yatc_io_fileExists(const char* fn);
char* yatc_io_readAll(const char* fn);
unsigned yatc_io_fileOutput(const char* fn, const char* what);

#endif