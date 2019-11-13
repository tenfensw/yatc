#ifndef YATC_VMIO
#define YATC_VMIO

void yatc_io_simplestOutput(const char* text);
char* yatc_io_prompt(const char* what);
unsigned yatc_io_unlink(const char* fn);
unsigned yatc_io_fileExists(const char* fn);
char* yatc_io_readAll(const char* fn);

#endif