#ifndef YATC_VMIO
#define YATC_VMIO

void yatc_io_simplestOutput(const char* text);
unsigned yatc_io_fileExists(const char* fn);
char* yatc_io_readAll(const char* fn);

#endif