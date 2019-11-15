#include "vmexpr.h"

int main(int argc, const char** argv) {
  char* expr = NULL;
  if (argc > 1) {
    expr = calloc(strlen(argv[1]) + 1, sizeof(char));
    strcpy(expr, argv[1]);
  } else {
    expr = calloc(6, sizeof(char));
    strcpy(expr, "2 < 2");
  }
  unsigned result = yatc_expressions_conditionMet(expr);
  free(expr);
  printf("%d\n", result);
  return 0;
}