#include "vm.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s FILENAME [arguments]\n", argv[0]);
    return 1;
  }
  const char* filepath = argv[1];
  if (!yatc_io_fileExists(filepath)) {
    fprintf(stderr, "'%s' - no such file or directory, cannot continue\n", filepath);
    return 2;
  }
  char* buf = yatc_io_readAll(filepath);
  YatcInterpreter* intpr = yatc_interpreter_create(NULL);
  YatcInterpreterResult* res = yatc_interpreter_exec(intpr, buf);
  free(buf);
  if (res && !(res->success)) {
    fprintf(stderr, "%s:%d - Error: %s\n", filepath, res->line, res->description);
    return 3;
  }
  free(res);
  yatc_interpreter_goodbye(intpr);
  return 0;
}
