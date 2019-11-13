#include <string.h>
#include "vmexpr.h"

int yatc_expressions_feedMath(const char* stringified, unsigned* ok) {
  unsigned whatToSetToOK = 0;
  if (!stringified) {
    if (ok) 
      *(ok) = 0;
    return -1;
  }
  int errst = 0;
  double resultOrig = te_interp(stringified, &errst);
  int result = -1;
  if (resultOrig != NAN) {
    whatToSetToOK = 1;
    result = (int)(resultOrig);
  }
  if (ok)
    *(ok) = whatToSetToOK;
  return result;
}