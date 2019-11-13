#include "vmcommon.h"
#include <stdio.h>

int main() {
  YatcVariable** vrbls = yatc_context_create(256);
  printf("Allocated at %p, current length = %d\n", vrbls, yatc_context_length(vrbls));
  char* meowieString = calloc(256, sizeof(char));
  strcpy(meowieString, "Meow!");
  YatcVariable* vl1 = yatc_variable_create("xiaomao", YString, meowieString);
  yatc_context_register(vrbls, vl1);
  yatc_context_fprintf(stdout, vrbls);
  yatc_context_goodbye(vrbls);
  return 0;
}