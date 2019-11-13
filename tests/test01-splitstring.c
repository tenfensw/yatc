#include <stdio.h>
#include "cext.h"

int main(int argc, const char** argv) {
   char* str = calloc(256, sizeof(char));
   if (argc < 2)
     strcpy(str, "if {$a == \"hello\"} { puts {Goodbye} }");
   else
     strcpy(str, argv[1]);
   printf("Original: %s\n", str);
   printf("Split one:\n");
   char** splitBySpaces = yatc_cstring_split(str, ' ');
   yatc_csarray_printf(splitBySpaces);
   free(splitBySpaces);
   return 0;
}
